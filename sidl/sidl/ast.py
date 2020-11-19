from typing import List, Optional, Tuple


class AstNode:
    position: Tuple[int, int]

    def __init__(self, pos: Tuple[int, int] = (0, 0)) -> None:
        self.position = pos

    def accept(self, visitor):
        visit_fn = f"visit_{type(self).__name__}"

        if hasattr(visitor, visit_fn):
            fn = getattr(visitor, visit_fn)
            fn(self)
        else:
            raise Exception(f"Visitor does not implement {visit_fn}")


class Type(AstNode):
    value: str

    def __init__(self, value: str) -> None:
        super().__init__()
        self.value = value


class Symbol(AstNode):
    value: str

    def __init__(self, value: str) -> None:
        super().__init__()
        self.value = value


class VariableDeclaration(AstNode):
    type: Type
    name: Symbol

    def __init__(self, type: Type, name: Symbol) -> None:
        super().__init__()
        self.type = type
        self.name = name


class Method(AstNode):
    name: Symbol
    arguments: List[VariableDeclaration]
    return_values: Optional[List[VariableDeclaration]]

    def __init__(self, name: Symbol) -> None:
        super().__init__()
        self.name = name
        self.arguments = []
        self.return_values = None

    def add_argument(self, argument: VariableDeclaration) -> None:
        self.arguments.append(argument)

    def add_return_value(self, retval: VariableDeclaration) -> None:
        if self.return_values is None:
            self.return_values = [retval]
        else:
            self.return_values.append(retval)


class Struct(AstNode):
    name: Symbol
    fields: List[VariableDeclaration]

    def __init__(self, name: Symbol) -> None:
        super().__init__()
        self.name = name
        self.fields = []

    def add_variable(self, var_type: Type, var_name: Symbol) -> None:
        self.fields.append(VariableDeclaration(var_type, var_name))


class Interface(AstNode):
    name: Symbol
    methods: List[Method]

    def __init__(self, name: Symbol) -> None:
        super().__init__()
        self.name = name
        self.methods = []

    def add_method(self, meth: Method) -> None:
        self.methods.append(meth)


class Namespace(AstNode):
    name: Symbol
    elements: List[AstNode]

    def __init__(self, name: Symbol) -> None:
        self.name = name
        self.elements = []

    def add_element(self, elem: AstNode) -> None:
        self.elements.append(elem)


class Visitor:
    def __init__(self):
        pass

    def visit_Type(self, node: Type) -> None:
        pass

    def visit_Symbol(self, node: Symbol) -> None:
        pass

    def visit_VariableDeclaration(self, node: VariableDeclaration) -> None:
        node.type.accept(self)
        node.name.accept(self)

    def visit_Method(self, node: Method) -> None:
        for argument in node.arguments:
            argument.accept(self)

        if node.return_values is not None:
            for retval in node.return_values:
                retval.accept(self)

    def visit_Struct(self, node: Struct) -> None:
        node.name.accept(self)

        for field in node.fields:
            field.type.accept(self)
            field.name.accept(self)

    def visit_Interface(self, node: Interface) -> None:
        node.name.accept(self)

        for method in node.methods:
            method.accept(self)

    def visit_Namespace(self, node: Namespace) -> None:
        node.name.accept(self)

        for elem in node.elements:
            elem.accept(self)

    def visit(self, root: AstNode) -> None:
        root.accept(self)
