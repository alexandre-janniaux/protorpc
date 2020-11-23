from typing import Dict
from sidl.ast import Visitor, Interface, Struct, Type


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
        if node.value not in self._defined_types:
            raise Exception(f"Unknown type: {node.value}")
        # Now resolve the type to its cpp equivalent

    def visit_Struct(self, node: Struct) -> None:
        struct_name = node.name.value

        if struct_name in self._defined_types:
            raise Exception(f"Redefinition of type: {struct_name}")

        for field in node.fields:
            field.accept(self)

        self._defined_types[struct_name] = struct_name
