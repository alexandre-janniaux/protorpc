from sidl.lexer import TokenType, Token, Lexer
from sidl.ast import AstNode


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

    def _parse_namespace(self) -> AstNode:
        pass

    def parse(self) -> AstNode:
        """ Toplevel node must be a namespace """
        pass
