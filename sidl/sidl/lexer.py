from typing import Tuple, Dict, Optional
from enum import Enum


class TokenType(Enum):
    Eof = 0
    Namespace = 1
    Interface = 2
    Struct = 3
    Arrow = 4
    LBrack = 5
    RBrack = 6
    LParen = 7
    RParen = 8
    LGeneric = 9
    RGeneric = 10
    Comma = 11
    Semicolon = 12
    Symbol = 13


class Position:
    line: int
    col: int

    def __init__(self, line: int = 1, col: int = 1) -> None:
        self.line = line
        self.col = col

    def __str__(self) -> str:
        return f"Position(line={self.line}, column={self.col})"

    def __repr__(self) -> str:
        return str(self)


class Token:
    type: TokenType
    value: str
    position: Position

    def __init__(
        self, tktype: TokenType, tkval: str, pos: Optional[Position] = None
    ) -> None:
        self.type = tktype
        self.value = tkval

        if pos:
            self.position = pos
        else:
            self.position = Position()

    def __str__(self):
        return self.value

    def __repr__(self):
        return f"Token(type={str(self.type)}, value='{self.value}', position={self.position})"


class Lexer:
    _data: str
    _pos: int
    _col: int
    _line: int
    _cur_token: Optional[Token]

    def __init__(self, data: str) -> None:
        self._data = data
        self._pos = 0
        self._col = 1
        self._line = 1
        self._cur_token = None

    def _skip_ws(self) -> bool:
        """ Eats whitespaces + comments and returns whether eof was reached. """
        in_comment = False

        while self._pos < len(self._data):
            c = self._data[self._pos]

            if c == '#' and not in_comment:
                in_comment = True

            if not c.isspace() and not in_comment:
                break

            if c == "\n":
                self._col = 1
                self._line += 1
                in_comment = False
            if c in [" ", "\r", "\t"] or in_comment:
                self._col += 1

            self._pos += 1

        return self._pos >= len(self._data)

    def reset(self) -> None:
        """ Resets the lexer to its original state """
        self._pos = 0
        self._col = 1
        self._line = 1
        self.cur_token = None

    def peek(self) -> Token:
        if self._cur_token:
            return self._cur_token

        self._cur_token = self.next()
        return self._cur_token

    def next(self) -> Token:
        if self._cur_token:
            tok = self._cur_token
            self._cur_token = None
            return tok

        tkval = ""
        tktype = TokenType.Eof

        if self._skip_ws():
            return Token(TokenType.Eof, "")


        separators: Dict[str, TokenType] = {
            "{": TokenType.LBrack,
            "}": TokenType.RBrack,
            "(": TokenType.LParen,
            ")": TokenType.RParen,
            "<": TokenType.LGeneric,
            ">": TokenType.RGeneric,
            ",": TokenType.Comma,
            ";": TokenType.Semicolon,
        }

        cur_line = self._line
        cur_col = self._col

        while self._pos < len(self._data):
            c = self._data[self._pos]

            if c.isspace():
                break

            # Arrow operator
            if c == ">" and tkval == "-":
                tkval = "->"
                tktype = TokenType.Arrow
                self._pos += 1
                self._col += 1
                break

            if c in separators:
                # If we reached a separator after word
                if len(tkval) > 0:
                    break

                # Otherwise we process the separator
                tkval = c
                tktype = separators[c]
                self._pos += 1
                self._col += 1
                break

            # Otherwise we have a normal character
            tkval += c
            self._pos += 1
            self._col += 1

        # Early return for separators and arrow
        if tktype != TokenType.Eof:
            return Token(tktype, tkval, Position(cur_line, cur_col))

        keywords: Dict[str, TokenType] = {
            "namespace": TokenType.Namespace,
            "interface": TokenType.Interface,
            "struct": TokenType.Struct,
        }

        tktype = keywords.get(tkval, TokenType.Symbol)

        return Token(tktype, tkval, Position(cur_line, cur_col))
