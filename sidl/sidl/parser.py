from typing import List
from sidl.lexer import TokenType, Token, Lexer
from sidl.ast import AstNode, Namespace, Interface, Symbol, Type, VariableDeclaration, Method, Struct


class ParsingException(Exception):
    message: str
    line: int
    col: int

    def __init__(self, message: str, line: int, col: int) -> None:
        self.message = message
        self.line = line
        self.col = col

    def __str__(self) -> str:
        return f"Parsing error at {self.line}:{self.col} : {self.message}"


class Parser:
    _lexer: Lexer

    def __init__(self, lexer: Lexer) -> None:
        self._lexer = lexer

    def _eof_next(self) -> Token:
        """ Returns the next token and raises an exception on eof """
        token = self._lexer.next()

        if token.type == TokenType.Eof:
            raise ParsingException("Unexpected eof", token.position.line,
                    token.position.col)

        return token

    def _eof_peek(self) -> Token:
        token = self._lexer.peek()

        if token.type == TokenType.Eof:
            raise ParsingException("Unexpected eof", token.position.line,
                    token.position.col)

        return token

    def _parse_var_decl(self) -> VariableDeclaration:
        var_type = self._eof_next()

        if var_type.type != TokenType.Symbol:
            raise ParsingException(f"Expected a symbol but got '{var_type.value}'",
                    var_type.position.line, var_type.position.col)

        var_name = self._eof_next()

        if var_name.type != TokenType.Symbol:
            raise ParsingException(f"Expected a symbol but got '{var_name.value}'",
                    var_name.position.line, var_name.position.col)

        arg_type = Type(var_type.value)
        arg_name = Symbol(var_name.value)

        return VariableDeclaration(arg_type, arg_name)

    def _parse_variable_pack(self) -> List[VariableDeclaration]:
        """
        Parses (Type1 var1, Type2 var2, ...)
        """
        lparent_tok = self._eof_next()

        if lparent_tok.type != TokenType.LParen:
            raise ParsingException(f"Expected '(' but got '{lparent_tok.value}'",
                    lparent_tok.position.line, lparent_tok.position.col)

        variable_pack: List[VariableDeclaration] = []

        while True:
            next_tok = self._eof_peek()

            if next_tok.type == TokenType.RParen:
                self._lexer.next()
                break

            variable_pack.append(self._parse_var_decl())
            comma_tok = self._eof_peek()

            if comma_tok.type == TokenType.Comma:
                self._lexer.next()
            elif comma_tok.type == TokenType.RParen:
                self._lexer.next()
                break

        return variable_pack

    def parse_method(self) -> Method:
        """
        unidirectional message: fn(args...);
        bidirectional request : fn(args...) -> (return values...);
        """
        name_tok = self._eof_next()

        if name_tok.type != TokenType.Symbol:
            raise ParsingException(f"Expected function name but got '{name_tok.value}'",
                    name_tok.position.line, name_tok.position.col)

        m = Method(Symbol(name_tok.value))
        m.position = (name_tok.position.line, name_tok.position.col)
        m.arguments = self._parse_variable_pack()

        arrow_tok = self._lexer.peek()

        if arrow_tok.type == TokenType.Eof or arrow_tok.type != TokenType.Arrow:
            return m

        if arrow_tok.type != TokenType.Arrow:
            raise ParsingException(f"Expected '->' but got '{arrow_tok.value}'",
                    arrow_tok.position.line, arrow_tok.position.col)

        self._lexer.next()
        m.return_values = self._parse_variable_pack()

        return m

    def parse_struct(self) -> Struct:
        struct_tok = self._eof_next()

        if struct_tok.type != TokenType.Struct:
            raise ParsingException(f"Expected 'struct' but got '{struct_tok.value}'",
                    struct_tok.position.line, name_tok.position.col)

        name_tok = self._eof_next()

        if name_tok.type != TokenType.Symbol:
            raise ParsingException(f"Expected struct name but got '{name_tok.value}'",
                    name_tok.position.line, name_tok.position.col)

        lbrack_tok = self._eof_next()

        if lbrack_tok.type != TokenType.LBrack:
            raise ParsingException(f"Expected '{{' but got '{lbrack_tok.value}'",
                    lbrack_tok.position.line, lbrack_tok.position.col)

        structure = Struct(Symbol(name_tok.value))
        structure.position = (struct_tok.position.line, struct_tok.position.col)

        while True:
            field_tok = self._eof_peek()

            if field_tok.type == TokenType.RBrack:
                self._lexer.next()
                break

            structure.fields.append(self._parse_var_decl())
            next_tok = self._eof_next()

            if next_tok.type != TokenType.Semicolon:
                raise ParsingException(f"Expected ';' but go '{next_tok.value}'",
                        next_tok.position.line, next_tok.position.col)

            next_tok = self._eof_peek()

            if next_tok.type == TokenType.RBrack:
                self._lexer.next()
                break

        return structure

    def parse_interface(self) -> Interface:
        int_tok = self._eof_next()

        if int_tok.type != TokenType.Interface:
            raise ParsingException(f"Expected 'interface' but got '{int_tok.value}'",
                    int_tok.position.line, int_tok.position.col)

        int_tok_name = self._eof_next()

        if int_tok_name.type != TokenType.Symbol:
            raise ParsingException(f"Expected '{TokenType.Symbol}' but got '{int_tok_name.type}'",
                    int_tok_name.position.kine, int_tok_name.position.col)

        lbrack_tok = self._eof_next()

        if lbrack_tok.type != TokenType.LBrack:
            raise ParsingException(f"Expected '{{' but got '{lbrack_tok.value}'",
                    lbrack_tok.position.line, lbrack_tok.position.col)

        intf = Interface(Symbol(int_tok_name.value))
        intf.position = (int_tok.position.line, int_tok.position.col)

        while True:
            field_tok = self._eof_peek()

            if field_tok.type == TokenType.RBrack:
                self._lexer.next()
                break

            intf.add_method(self.parse_method())
            next_tok = self._eof_next()

            if next_tok.type != TokenType.Semicolon:
                raise ParsingException(f"Expected ';' but go '{next_tok.value}'",
                        next_tok.position.line, next_tok.position.col)

            next_tok = self._eof_peek()

            if next_tok.type == TokenType.RBrack:
                self._lexer.next()
                break

        return intf

    def parse_namespace(self) -> AstNode:
        ns_tok = self._eof_next()

        if ns_tok.type != TokenType.Namespace:
            raise ParsingException(f"Expected 'namespace' but got '{ns_tok.value}'",
                    ns_tok.position.line, ns_tok.position.col)

        ns_name_tok = self._eof_next()

        if ns_name_tok.type != TokenType.Symbol:
            raise ParsingException(f"Expected {TokenType.Symbol} but got {ns_tok.type}",
                    ns_name_tok.position.line, ns_name_tok.position.col)

        ns = Namespace(Symbol(ns_name_tok.value))
        ns.position = (ns_tok.position.line, ns_tok.position.col)

        lbrack_tok = self._eof_next()

        if lbrack_tok.type != TokenType.LBrack:
            raise ParsingException(f"Expected '{{' but got '{lbrack_tok.value}'",
                    lbrack_tok.position.line, lbrack_tok.position.col)

        while True:
            field_tok = self._eof_peek()

            if field_tok.type == TokenType.RBrack:
                self._lexer.next()
                break

            if field_tok.type == TokenType.Namespace:
                ns.add_element(self.parse_namespace())
            elif field_tok.type == TokenType.Struct:
                ns.add_element(self.parse_struct())
            elif field_tok.type == TokenType.Interface:
                ns.add_element(self.parse_interface())
            else:
                raise ParsingException(f"Expected 'struct' or 'interface' but got '{field_tok.value}'",
                        field_tok.position.line, field_tok.position.col)

        return ns

    def parse(self) -> AstNode:
        """ Toplevel node must be a namespace """
        self._lexer.reset()
        return self.parse_namespace()
