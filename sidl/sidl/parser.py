from sidl.lexer import TokenType, Token, Lexer
from sidl.ast import AstNode, Namespace, Interface, Symbol


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

    def parse_method(self) -> AstNode:
        pass

    def parse_interface(self) -> AstNode:
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

            if field_tok.type != TokenType.LParen:
                raise ParsingException("Expected interface method '(...) -> (...);' but got '{field_tok.value}'",
                        field_tok.position.line, field_tok.position.col)

            # TODO: Parse method

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
                print("parse struct")
            elif field_tok.type == TokenType.Interface:
                print("parse interface")
            else:
                raise ParsingException("Expected 'struct' or 'interface' but got '{field_tok.value}'",
                        field_tok.position.line, field_tok.position.col)

        return ns

    def parse(self) -> AstNode:
        """ Toplevel node must be a namespace """
        return self.parse_namespace()
