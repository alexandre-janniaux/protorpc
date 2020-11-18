from sidl.ast import Visitor, VariableDeclaration, Symbol, Type, Method, Interface, Namespace
from sidl.utils import PrettyPrinter


def main():
    namespace = Namespace(Symbol("vlc"))
    vlc_stream = Interface(Symbol("VlcStream"))

    offset_method = Method(Symbol("offset"))
    offset_method.add_argument(VariableDeclaration(Type("off_t"), Symbol("offset")))
    offset_method.add_return_value(VariableDeclaration(Type("int"), Symbol("err")))

    vlc_stream.add_method(offset_method)

    namespace.add_element(vlc_stream)

    pprinter = PrettyPrinter()
    pprinter.visit(namespace)

    print(pprinter.data)


if __name__ == "__main__":
    main()
