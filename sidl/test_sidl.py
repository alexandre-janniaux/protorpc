import pytest
from sidl.lexer import TokenType, Token, Lexer


def test_parse_simple_1():
    lex = Lexer("method_name (i32 a, i32\tb)    \n -> (string result);")

    expected = [
        (TokenType.Symbol, "method_name"),
        (TokenType.LParen, "("),
        (TokenType.Symbol, "i32"),
        (TokenType.Symbol, "a"),
        (TokenType.Comma, ","),
        (TokenType.Symbol, "i32"),
        (TokenType.Symbol, "b"),
        (TokenType.RParen, ")"),
        (TokenType.Arrow, "->"),
        (TokenType.LParen, "("),
        (TokenType.Symbol, "string"),
        (TokenType.Symbol, "result"),
        (TokenType.RParen, ")"),
        (TokenType.Semicolon, ";"),
        (TokenType.Eof, ""),
        (TokenType.Eof, ""),
    ]

    for tktype, tkval in expected:
        tok = lex.next()

        assert tok.type == tktype
        assert tok.value == tkval


def test_parse_simple_1_nospaces():
    lex = Lexer("method_name(i32 a,i32 b)->(string result);")

    expected = [
        (TokenType.Symbol, "method_name"),
        (TokenType.LParen, "("),
        (TokenType.Symbol, "i32"),
        (TokenType.Symbol, "a"),
        (TokenType.Comma, ","),
        (TokenType.Symbol, "i32"),
        (TokenType.Symbol, "b"),
        (TokenType.RParen, ")"),
        (TokenType.Arrow, "->"),
        (TokenType.LParen, "("),
        (TokenType.Symbol, "string"),
        (TokenType.Symbol, "result"),
        (TokenType.RParen, ")"),
        (TokenType.Semicolon, ";"),
        (TokenType.Eof, ""),
        (TokenType.Eof, ""),
    ]

    for tktype, tkval in expected:
        tok = lex.next()

        assert tok.type == tktype
        assert tok.value == tkval


def test_lexer_peek_1():
    lex = Lexer("a b c")

    token = lex.peek()
    assert token.type == TokenType.Symbol
    assert token.value == "a"

    token = lex.peek()
    assert token.type == TokenType.Symbol
    assert token.value == "a"

    token = lex.next()
    assert token.type == TokenType.Symbol
    assert token.value == "a"

    token = lex.next()
    assert token.type == TokenType.Symbol
    assert token.value == "b"

    token = lex.peek()
    assert token.type == TokenType.Symbol
    assert token.value == "c"
