#!/usr/bin/env python3

import argparse
from sidl.lexer import Lexer
from sidl.parser import Parser
from sidl.compiler import CppSourceCompiler, CppHeaderCompiler
from sidl.utils import PrettyPrinter, SidlException
from sidl.proxy_compiler import ProxySourceCompiler
from sidl.type_resolver import TypeResolver


def main():
    parser = argparse.ArgumentParser(description="protorpc Small IDL compiler")
    parser.add_argument(
        "-o", "--outdir", help="Output directory for the generated files", default=""
    )
    parser.add_argument(
        "--header-only", help="Only generate the header file", action="store_true"
    )
    parser.add_argument(
        "--source-only", help="Only generate the cpp file", action="store_true"
    )
    parser.add_argument("idl_file", help="input idl file")

    args = parser.parse_args()

    # More setup
    compile_header = True
    compile_impl = True

    if args.header_only and args.source_only:
        pass
    elif args.header_only:
        compile_impl = False
    elif args.source_only:
        compile_header = False

    impl_path = "./" + args.outdir + "/" + args.idl_file + ".cpp"
    header_path = "./" + args.outdir + "/" + args.idl_file + ".hh"

    # XXX: Debug code
    data = open(args.idl_file, "r").read()
    lines = data.split("\n")
    lex = Lexer(data)
    p = Parser(lex)

    try:
        root = p.parse()

        tr = TypeResolver()
        tr.visit(root)

        if compile_impl:
            proxy_source_compiler = ProxySourceCompiler(args.idl_file, tr.types)
            proxy_source_compiler.visit(root)

            open(impl_path, "w").write(proxy_source_compiler.data)

        if compile_header:
            print("header compilation not implemented for now")
            pass

    except SidlException as err:
        start = max(err.line - 10, 0)

        for line_idx in range(start, err.line):
            print(lines[line_idx])

        print(" " * (err.col - 1) + "^")

        print(f"{args.idl_file}:{err.line}:{err.col}: {err.message}")


if __name__ == "__main__":
    main()
