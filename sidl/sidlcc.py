#!/usr/bin/env python3

import argparse

if __name__ == "__main__":
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
