from typing import Dict
from sidl.ast import Visitor, Interface, Struct, Type
from sidl.utils import SidlException


class TypeResolver(Visitor):
    """
    Type checks an idl (verifies that types exist). Modifies the ast to use C++ types
    instead of SIDL types
    """

    _defined_types: Dict[str, str]
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

        for field in node.fields:
            field.accept(self)

        self._defined_types[struct_name] = struct_name

    @property
    def types(self) -> Dict[str, str]:
        return self._defined_types
