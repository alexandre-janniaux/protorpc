from typing import List
from sidl.utils import IndentedWriter
from sidl.ast import (
    Visitor,
    VariableDeclaration,
    Type,
    Symbol,
    Method,
    Struct,
    Interface,
    Namespace,
)


class CppSourceCompiler(Visitor):
    _writer: IndentedWriter
    _current_interface: str

    def __init__(self, indent=4) -> None:
        self._writer = IndentedWriter(indent)

        # Name of the current object. It is used by the method visitor to know
        # to which class a method is attached.
        self._current_interface = ""

    def visit_Type(self, node: Type) -> None:
        type_conversion = {
            "bool": "bool",
            "u8": "std::uint8_t",
            "u16": "std::uint16_t",
            "u32": "std::uint32_t",
            "u64": "std::uint64_t",
            "i8": "std::int8_t",
            "i16": "std::int16_t",
            "i32": "std::int32_t",
            "i64": "std::int64_t",
            "string": "std::string",
        }

        cpp_type = type_conversion.get(node.value, node.value)
        self._writer.write(cpp_type)

    def visit_Symbol(self, node: Symbol) -> None:
        self._writer.write(node.value)

    def visit_VariableDeclaration(self, decl: VariableDeclaration) -> None:
        decl.type.accept(self)
        self._writer.write(" ")
        decl.name.accept(self)

    def visit_Method(self, node: Method) -> None:
        self._writer.write(f"rpc::Error {self._current_interface}::{node.name.value}(")

        for i, e in enumerate(node.arguments):
            e.accept(self)

            if i != len(node.arguments) - 1:
                self._writer.write(", ")

        if node.return_values:
            for e in node.return_values:
                self._writer.write(", ")
                e.type.accept(self)
                self._writer.write("* ")
                e.name.accept(self)

        self._writer.write_line(")")
        self._writer.write_line("{")
        self._writer.indent()
        self._writer.write_line("// TODO: Implement serialization code generation")
        self._writer.deindent()
        self._writer.write_line("}")

    def visit_Struct(self, node: Struct) -> None:
        # Structs are defined in the headers
        pass

    def visit_Interface(self, node: Interface) -> None:
        self._current_interface = node.name.value

        for method in node.methods:
            method.accept(self)

    def visit_Namespace(self, node: Namespace) -> None:
        self._writer.write_line(f"namespace {node.name.value}")
        self._writer.write_line("{")
        self._writer.indent()

        for elem in node.elements:
            elem.accept(self)

        self._writer.deindent()
        self._writer.write_line("}")

    @property
    def data(self) -> str:
        return self._writer.data()


class CppHeaderCompiler(Visitor):
    def __init__(self, indent=4) -> None:
        self._writer = IndentedWriter(indent)

    def visit_Type(self, node: Type) -> None:
        type_conversion = {
            "bool": "bool",
            "u8": "std::uint8_t",
            "u16": "std::uint16_t",
            "u32": "std::uint32_t",
            "u64": "std::uint64_t",
            "i8": "std::int8_t",
            "i16": "std::int16_t",
            "i32": "std::int32_t",
            "i64": "std::int64_t",
            "string": "std::string",
        }

        cpp_type = type_conversion.get(node.value, node.value)
        self._writer.write(cpp_type)

    def visit_Symbol(self, node: Symbol) -> None:
        self._writer.write(node.value)

    def visit_VariableDeclaration(self, decl: VariableDeclaration) -> None:
        decl.type.accept(self)
        self._writer.write(" ")
        decl.name.accept(self)

    def visit_Method(self, node: Method) -> None:
        self._writer.write(f"rpc::Error {node.name.value}(")

        for i, e in enumerate(node.arguments):
            e.accept(self)

            if i != len(node.arguments) - 1:
                self._writer.write(", ")

        if node.return_values:
            for e in node.return_values:
                self._writer.write(", ")
                e.type.accept(self)
                self._writer.write("& ")
                e.name.accept(self)

        self._writer.write_line(");")

    def visit_Struct(self, node: Struct) -> None:
        self._writer.write_line(f"struct {node.name.value}")
        self._writer.write_line("{")
        self._writer.indent()

        for field in node.fields:
            field.accept(self)
            self._writer.write_line(";")

        self._writer.deindent()
        self._writer.write_line("};")

    def visit_Interface(self, node: Interface) -> None:
        self._writer.write_line(f"class {node.name.value}")
        self._writer.write_line("{")
        self._writer.write_line("public:")
        self._writer.indent()

        for method in node.methods:
            method.accept(self)

        self._writer.deindent()
        self._writer.write_line("};")

    def visit_Namespace(self, node: Namespace) -> None:
        self._writer.write_line(f"namespace {node.name.value}")
        self._writer.write_line("{")
        self._writer.indent()

        for elem in node.elements:
            elem.accept(self)

        self._writer.deindent()
        self._writer.write_line("}")

    @property
    def data(self) -> str:
        return self._writer.data()
