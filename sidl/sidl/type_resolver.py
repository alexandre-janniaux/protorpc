from typing import Dict, Set
from sidl.ast import Visitor, Interface, Struct, Type, Method
from sidl.utils import SidlException


class TypeResolver(Visitor):
    """
    Type checks an idl (verifies that types exist). Modifies the ast to use C++ types
    instead of SIDL types
    """

    _defined_types: Dict[str, str]
    _defined_interfaces: Set[str]
    _defined_methods: Set[str]
    _handle_tainted: Set[str]
    _type_depth: int

    def __init__(self):
        self._defined_types = {
            "bool": "bool",
            "u8": "std::uint8_t",
            "u16": "std::uint16_t",
            "u32": "std::uint32_t",
            "u64": "std::uint64_t",
            "i8": "std::int8_t",
            "i16": "std::int16_t",
            "i32": "std::int32_t",
            "i64": "std::int64_t",
            "usize": "std::size_t",
            "string": "std::string",
            "handle": "int",
            "optional": "std::optional",
            "vec": "std::vector",
        }

        # Current depth inside the type (used for generic types).
        self._type_depth = 0
        self._defined_interfaces = set()
        self._defined_methods = set()

        # Set of structures containing handles. These structures cannot be put
        # inside containers.
        self._handle_tainted = set()

    def visit_Type(self, node: Type) -> None:
        # Container types and their number of arguments
        container_types = {
            "vec": 1,
            "optional": 1
        }

        if node.value not in self._defined_types:
            raise SidlException(f"Unknown type: {node.value}", *node.position)

        if len(node.generics) and node.value not in container_types:
            raise SidlException(f"Type {node.value} is not a generic container", *node.position)

        # Handle is a special type and cannot be contained
        if node.value == "handle" and self._type_depth > 0:
            raise SidlException("Handle type cannot be contained", *node.position)

        if node.value in self._handle_tainted and self._type_depth > 0:
            raise SidlException("Struct cannot be contained because it contains handles",
                    *node.position)

        # Checking container argument arity
        if node.value in container_types:
            expected = container_types[node.value]

            if len(node.generics) != expected:
                raise SidlException(f"Expected {expected} generic type arguments but {len(node.generics)} where provided",
                        *node.position)

        self._type_depth += 1

        for ty in node.generics:
            ty.accept(self)

        self._type_depth -= 1

    def visit_Struct(self, node: Struct) -> None:
        struct_name = node.name.value

        if struct_name in self._defined_types:
            raise SidlException(f"Redefinition of type: {struct_name}", *node.position)

        defined_fields: Set[str] = set()

        for field in node.fields:
            field_name = field.name.value
            field_type = field.type.value

            if field_name in defined_fields:
                raise SidlException(f"Redefinition of struct field: {field_name}",
                        *field.name.position)

            if field_type == "handle" or field_type in self._handle_tainted:
                self._handle_tainted.add(struct_name)

            defined_fields.add(field_name)
            field.accept(self)

        self._defined_types[struct_name] = struct_name

    def visit_Interface(self, node: Interface) -> None:
        intf_name = node.name.value

        if intf_name in self._defined_types:
            raise SidlException(f"Redefinition of type to interface: {intf_name}", *node.position)

        if intf_name in self._defined_interfaces:
            raise SidlException(f"Redefinition of interface: {intf_name}", *node.position)

        self._defined_interfaces.add(intf_name)
        self._defined_methods = set()

        super().visit_Interface(node)

    def visit_Method(self, node: Method) -> None:
        method_name = node.name.value

        if method_name in self._defined_methods:
            raise SidlException(f"Redefinition of interface method: {method_name}", *node.position)

        defined_names: Set[str] = set()

        for arg in node.arguments:
            arg_name = arg.name.value

            if arg_name in defined_names:
                raise SidlException(f"Redefinition of argument: {arg_name}", *arg.name.position)

            defined_names.add(arg.name.value)

        if node.return_values:
            for ret in node.return_values:
                ret_name = ret.name.value

                if ret_name in defined_names:
                    raise SidlException(f"Redefinition of method argument: {ret_name}",
                            *ret.name.position)

                defined_names.add(ret_name)

    @property
    def types(self) -> Dict[str, str]:
        return self._defined_types
