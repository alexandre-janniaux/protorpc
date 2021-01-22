from typing import Dict, Set, List
from sidl.utils import IndentedWriter
from sidl.ast import Visitor, Type, Symbol, VariableDeclaration, Namespace, AstNode, Struct, VariableDeclaration, Interface


def fixup_struct_types(root: AstNode) -> Dict[str, str]:
    """
    Struct names are left as is after the type resolving. We need to prefix
    them with the namespace information.
    """

    class StructCollector(Visitor):
        namespace: List[str]

        def __init__(self):
            self.namespace = []

        def visit_Namespace(self, node: Namespace) -> None:
            self.namespace.append(node.name.value)

            for elem in node.elements:
                elem.accept(self)

            self.namespace.pop()

class BaseCCompiler(Visitor):
    """
    Common code shared between the C visitors.
    """

    writer: IndentedWriter
    _types: Dict[str, str]
    _core_types: Set[str]
    _namespace_parts: List[str]

    def __init__(self, types: Dict[str, str], indent: int = 4) -> None:
        self._types = types
        self._core_types = set(["bool", "u8", "u16", "u32", "u64", "i8", "i16", "i32", "i64", "usize"])
        self._namespace_parts = []
        self.writer = IndentedWriter(indent)

    def cify_type(self, node: Type) -> str:
        if node.value == "vec":
            return "sidl_" + self.cify_type(node.generics[0]) + "_vector"
        elif node.value == "optional":
            return "sidl_" + self.cify_type(node.generics[0]) + "_optional"
        else:
            return node.value

    def visit_Type(self, node: Type) -> None:
        c_type = self._types.get(node.value, node.value)
        self.writer.write(self.cify_type(node))

    def visit_Symbol(self, node: Symbol) -> None:
        self.writer.write(node.value)

    def visit_VariableDeclaration(self, decl: VariableDeclaration) -> None:
        decl.type.accept(self)
        self.writer.write(" ")
        decl.name.accept(self)

    def visit_Namespace(self, node: Namespace) -> None:
        self._namespace_parts.append(node.name.value)

        for elem in node.elements:
            elem.accept(self)

        self._namespace_parts.pop()

    def namespace_prefix(self) -> str:
        if len(self._namespace_parts) == 0:
            return ""

        return "_".join(self._namespace_parts) + "_"

    @property
    def data(self) -> str:
        return self.writer.data()


class HeaderCompiler(BaseCCompiler):
    _filename: str
    _types: Dict[str, str]

    def __init__(self, filename: str, types: Dict[str, str], indent: int = 4) -> None:
        super().__init__(types, indent)

        self._filename = filename
        self._types = types

    def visit_Interface(self, node: Interface) -> None:
        pass

    def visit_VariableDeclaration(self, node: VariableDeclaration) -> None:
        node.type.accept(self)
        self.writer.write(" ")
        node.name.accept(self)

    def visit_Struct(self, node: Struct) -> None:
        struct_name = self.namespace_prefix() + node.name.value

        self.writer.write_line(f"typedef struct {struct_name}")
        self.writer.write_line("{")
        self.writer.indent()

        for field in node.fields:
            field.accept(self)
            self.writer.write_line(";")

        self.writer.deindent()
        self.writer.write_line(f"}} {struct_name};")

        self.writer.write_line("")
        self.writer.write_line(f"void {struct_name}_destroy({struct_name}* obj);")
        self.writer.write_line(f"int {struct_name}_read(sidl_unserializer_t* u, {struct_name}* obj);")
        self.writer.write_line(f"int {struct_name}_write(sidl_serializer_t* s, {struct_name}* obj);")

    def visit(self, root: AstNode) -> None:
        header_name = self._filename.replace(".", "_").upper() + "_H"
        self.writer.write_line(f"#ifndef {header_name}")
        self.writer.write_line(f"#define {header_name}")
        self.writer.write_line(f"#include \"cprotorpc/structures.h\"")
        self.writer.write_line(f"#include \"cprotorpc/unserializer.h\"")
        self.writer.write_line(f"#include \"cprotorpc/serializer.h\"")
        self.writer.write_line("")

        root.accept(self)

        self.writer.write_line("")
        self.writer.write_line("#endif")


class SourceCompiler(BaseCCompiler):
    _filename: str
    _types: Dict[str, str]

    def __init__(self, filename: str, types: Dict[str, str], indent: int = 4) -> None:
        super().__init__(types, indent)

        self._filename = filename
        self._types = types

    def visit_Interface(self, node: Interface) -> None:
        pass

    def _compile_type_destroy(self, ty: Type) -> None:
        pass

    def _compile_field_destructor(self, node: VariableDeclaration) -> None:
        pass

    def _compile_destroy_function(self, node: Struct) -> None:
        struct_name = self.namespace_prefix() + node.name.value

        self.writer.write_line(f"void {struct_name}_destroy({struct_name}* obj)")
        self.writer.write_line("{")
        self.writer.indent()

        self.writer.deindent()
        self.writer.write_line("}")

    def visit_Struct(self, node: Struct) -> None:
        self._compile_destroy_function(node)
