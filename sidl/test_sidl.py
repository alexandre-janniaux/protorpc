import pytest
from sidl.lexer import TokenType, Token, Lexer
from sidl.parser import Parser, SidlException
from sidl.utils import PrettyPrinter


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


def test_parse_method_1():
    idl_example = "test(t1 a, t2 b)"

    lex = Lexer(idl_example)
    p = Parser(lex)

    method = p.parse_method()

    assert method.name.value == "test"
    assert len(method.arguments) == 2
    assert method.return_values is None


def test_parse_method_2():
    idl_example = "test2 (string a, string b, string c) -> (int a, int b)"
    lex = Lexer(idl_example)
    p = Parser(lex)

    method = p.parse_method()

    assert method.name.value == "test2"
    assert len(method.arguments) == 3
    assert method.return_values is not None
    assert len(method.return_values) == 2


def test_parse_struct_1():
    idl_example = """
    struct Simple {
        u64 a;
        string b;
        handle c;
    }
    """

    lex = Lexer(idl_example)
    p = Parser(lex)

    struct = p.parse_struct()

    assert len(struct.fields) == 3
    assert struct.name.value == "Simple"


def test_parse_interface_1():
    idl_example = """
    interface File {
        read(usize offset, usize count) -> (string data);
        close() -> ();
    }
    """

    lex = Lexer(idl_example)
    p = Parser(lex)

    intf = p.parse_interface()

    assert len(intf.methods) == 2
    assert intf.name.value == "File"


def test_parse_interface_2():
    idl_example = """
    interface File {
        close();
    }
    """

    lex = Lexer(idl_example)
    p = Parser(lex)

    intf = p.parse_interface()

    assert len(intf.methods) == 1
    close = intf.methods[0]
    assert close.return_values is None


def test_parse_generics_1():
    idl_example = "vec<optional<i64>>"

    lex = Lexer(idl_example)
    p = Parser(lex)

    ty = p.parse_type()

    assert ty.value == "vec"
    assert len(ty.generics) == 1
    assert ty.generics[0].value == "optional"
    assert len(ty.generics[0].generics) == 1
    assert ty.generics[0].generics[0].value == "i64"


def test_parse_generics_2():
    idl_example = "map<key, vec<map<k,v>>>"

    lex = Lexer(idl_example)
    p = Parser(lex)

    ty = p.parse_type()

    assert ty.value == "map"
    assert len(ty.generics) == 2

    assert ty.generics[0].value == "key"
    assert len(ty.generics[1].generics) == 1

    m = ty.generics[1].generics[0]

    assert m.value == "map"
    assert len(m.generics) == 2
    assert m.generics[0].value == "k"
    assert m.generics[1].value == "v"


def test_parse_generics_3():
    idl_example = "T<>"

    lex = Lexer(idl_example)
    p = Parser(lex)

    ty = p.parse_type()

    assert ty.value == "T"
    assert len(ty.generics) == 0


def test_parse_generics_4():
    idl_example = "T<A,>"

    lex = Lexer(idl_example)
    p = Parser(lex)

    with pytest.raises(SidlException):
        p.parse_type()
