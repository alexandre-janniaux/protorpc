#!/usr/bin/env python3

import argparse
import os
from sidl.lexer import Lexer
from sidl.parser import Parser
from sidl.utils import PrettyPrinter, SidlException
from sidl.compiler import CppSourceCompiler, CppHeaderCompiler, CSourceCompiler, CHeaderCompiler
from sidl.type_resolver import CppTypeResolver, CTypeResolver


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
    parser.add_argument(
        "-b", "--backend", help="Compilation backend (c, cpp)", default="cpp"
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

    if args.backend not in ["c", "cpp"]:
        print(f"Invalid backend '{parser.backend}'")
        return

    idl_filename = os.path.basename(args.idl_file)

    data = open(args.idl_file, "r").read()
    lines = data.split("\n")
    lex = Lexer(data)
    p = Parser(lex)

    try:
        root = p.parse()

        if args.backend == "cpp":
            impl_path = "./" + args.outdir + "/" + idl_filename + ".cpp"
            header_path = "./" + args.outdir + "/" + idl_filename + ".hh"

            tr = CppTypeResolver()
            tr.visit(root)

            if compile_impl:
                source_compiler = CppSourceCompiler(idl_filename, tr.types)
                source_compiler.visit(root)

                open(impl_path, "w").write(source_compiler.data)

            if compile_header:
                header_compiler = CppHeaderCompiler(idl_filename, tr.types)
                header_compiler.visit(root)

                open(header_path, "w").write(header_compiler.data)
        else:
            impl_path = "./" + args.outdir + "/" + idl_filename + ".c"
            header_path = "./" + args.outdir + "/" + idl_filename + ".h"

            print("WARNING: C backend is experimental and incomplete")
            tr = CTypeResolver()
            tr.visit(root)

            if compile_impl:
                source_compiler = CSourceCompiler(idl_filename, tr.types)
                source_compiler.visit(root)

                open(impl_path, "w").write(source_compiler.data)

            if compile_header:
                header_compiler = CHeaderCompiler(idl_filename, tr.types)
                header_compiler.visit(root)

                open(header_path, "w").write(header_compiler.data)

    except SidlException as err:
        start = max(err.line - 10, 0)

        for line_idx in range(start, err.line):
            print(lines[line_idx])

        print(" " * (err.col - 1) + "^")

        print(f"{args.idl_file}:{err.line}:{err.col}: {err.message}")


if __name__ == "__main__":
    main()
