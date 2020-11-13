from sidl.ast import Visitor, VariableDeclaration, Symbol, Type, Method, Interface, Namespace


class PrettyPrinter(Visitor):
    def __init__(self, indent=4) -> None:
        self.indent_level = 0
        self.indent = indent
        self.data = ""

    def append(self, s) -> None:
        self.data += s

    def newline(self) -> None:
        self.data += "\n"
        self.data += " " * (self.indent_level * self.indent)

    def visit_Type(self, node: Type) -> None:
        self.append(node.value)

    def visit_Symbol(self, node: Symbol) -> None:
        self.append(node.value)

    def visit_VariableDeclaration(self, node: VariableDeclaration) -> None:
        node.type.accept(self)
        self.append(" ")
        node.name.accept(self)

    def visit_Method(self, node: Method) -> None:
        node.name.accept(self)
        self.append("(")

        for i, arg in enumerate(node.arguments):
            arg.accept(self)

            if i != len(node.arguments) - 1:
                self.append(", ")

        self.append(")")

        if node.return_values is not None:
            self.append(" -> ")
            self.append("(")

            for i, arg in enumerate(node.return_values):
                arg.accept(self)

                if i != len(node.return_values) - 1:
                    self.append(", ")

            self.append(")")

        self.append(";")

    def visit_Interface(self, node: Interface) -> None:
        self.append("interface ")
        node.name.accept(self)
        self.append(" {")

        if len(node.methods) > 0:
            self.indent_level += 1

        for meth in node.methods:
            self.newline()
            meth.accept(self)

        if len(node.methods) > 0:
            self.indent_level -= 1
            self.newline()

        self.append("}")

    def visit_Namespace(self, node: Namespace) -> None:
        self.append("namespace ")
        node.name.accept(self)
        self.append(" {")

        if len(node.elements) > 0:
            self.indent_level += 1

        for elem in node.elements:
            self.newline()
            elem.accept(self)

        if len(node.elements) > 0:
            self.indent_level -= 1
            self.newline()

        self.append("}")


def main():
    vlc_stream = Interface(Symbol("VlcStream"))

    offset_method = Method(Symbol("offset"))
    offset_method.add_argument(VariableDeclaration(Type("off_t"), Symbol("offset")))
    offset_method.add_return_value(VariableDeclaration(Type("int"), Symbol("err")))

    vlc_stream.add_method(offset_method)

    pprinter = PrettyPrinter()
    pprinter.visit(vlc_stream)

    print(pprinter.data)


if __name__ == "__main__":
    main()
