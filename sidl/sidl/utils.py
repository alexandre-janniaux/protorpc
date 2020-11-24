import io
from sidl.ast import (
    Visitor,
    VariableDeclaration,
    Symbol,
    Type,
    Method,
    Interface,
    Namespace,
    Struct
)


class SidlException(Exception):
    message: str
    line: int
    col: int

    def __init__(self, message: str, line: int, col: int) -> None:
        self.message = message
        self.line = line
        self.col = col

    def __str__(self) -> str:
        return f"Error at {self.line}:{self.col} : {self.message}"


class IndentedWriter:
    """
    Helper object for pretty printing.
    """
    _is_newline: bool
    _indentation: int

    def __init__(self, indentation: int = 4) -> None:
        self._is_newline = False
        self._indentation = indentation
        self._level = 0
        self._stream = io.StringIO()

    def data(self) -> str:
        return self._stream.getvalue()

    def _write_indent(self) -> None:
        if self._is_newline:
            self._stream.write((self._level * self._indentation) * " ")
            self._is_newline = False

    def _newline(self) -> None:
        self._stream.write("\n")
        self._is_newline = True

    def write(self, data: str) -> None:
        self._write_indent()
        self._stream.write(data)

    def write_line(self, data: str) -> None:
        self._write_indent()
        self._stream.write(data)
        self._newline()

    def indent(self) -> None:
        self._level += 1

    def deindent(self) -> None:
        assert self._level > 0
        self._level -= 1


class PrettyPrinter(Visitor):
    """
    Ast pretty printer
    """

    _writer: IndentedWriter

    def __init__(self, indent=4) -> None:
        self._writer = IndentedWriter(indent)

    def visit_Type(self, node: Type) -> None:
        self._writer.write(node.value)

    def visit_Symbol(self, node: Symbol) -> None:
        self._writer.write(node.value)

    def visit_VariableDeclaration(self, node: VariableDeclaration) -> None:
        node.type.accept(self)
        self._writer.write(" ")
        node.name.accept(self)

    def visit_Method(self, node: Method) -> None:
        node.name.accept(self)
        self._writer.write("(")

        for i, arg in enumerate(node.arguments):
            arg.accept(self)

            if i != len(node.arguments) - 1:
                self._writer.write(", ")

        self._writer.write(")")

        if node.return_values is not None:
            self._writer.write(" -> ")
            self._writer.write("(")

            for i, arg in enumerate(node.return_values):
                arg.accept(self)

                if i != len(node.return_values) - 1:
                    self._writer.write(", ")

            self._writer.write(")")

        self._writer.write_line(";")

    def visit_Struct(self, node: Struct) -> None:
        self._writer.write("struct ")
        node.name.accept(self)

        if len(node.fields) == 0:
            self._writer.write_line(" {}")
            return

        self._writer.write_line(" {")
        self._writer.indent()

        for field in node.fields:
            field.type.accept(self)
            self._writer.write(" ")
            field.name.accept(self)
            self._writer.write_line(";")

        self._writer.deindent()
        self._writer.write_line("}")

    def visit_Interface(self, node: Interface) -> None:
        self._writer.write("interface ")
        node.name.accept(self)

        if len(node.methods) == 0:
            self._writer.write_line(" {}")
            return

        self._writer.write_line(" {")
        self._writer.indent()

        for meth in node.methods:
            meth.accept(self)

        self._writer.deindent()
        self._writer.write_line("}")

    def visit_Namespace(self, node: Namespace) -> None:
        self._writer.write("namespace ")
        node.name.accept(self)

        if len(node.elements) == 0:
            self._writer.write_line(" {}")
            return

        self._writer.write_line(" {")
        self._writer.indent()

        for elem in node.elements:
            elem.accept(self)

        self._writer.deindent()
        self._writer.write_line("}")

    @property
    def data(self) -> str:
        return self._writer.data()
