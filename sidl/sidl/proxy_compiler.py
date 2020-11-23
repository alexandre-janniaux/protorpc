from sidl.utils import IndentedWriter
from sidl.ast import Visitor, Symbol, Method, Type, Interface, Namespace, VariableDeclaration, Struct


class ProxySourceCompiler(Visitor):
    _writer: IndentedWriter
    _current_interface: str
    _current_opcode: int

    def __init__(self, indent=4) -> None:
        self._writer = IndentedWriter(indent)
        self._current_opcode = 0

    def visit_Type(self, node: Type) -> None:
        type_conversion = {
            "bool": "bool",
            "u8": "std::uint8_t",
            "u16": "std::uint16_t",
            "u32": "std::uint32_t",
            "u64": "std::uint64_t",
            "i8": "std::int8_t",
            "i16": "std::int16_t",
            "i32": "std::int32_t",
            "i64": "std::int64_t",
            "string": "std::string",
        }

        cpp_type = type_conversion.get(node.value, node.value)
        self._writer.write(cpp_type)

    def visit_Symbol(self, node: Symbol) -> None:
        self._writer.write(node.value)

    def visit_VariableDeclaration(self, decl: VariableDeclaration) -> None:
        decl.type.accept(self)
        self._writer.write(" ")
        decl.name.accept(self)

    def visit_Struct(self, node: Struct) -> None:
        pass

    def visit_Method(self, node: Method) -> None:
        # prototype generation
        self._writer.write(f"bool {self._current_interface}::{node.name.value} (")

        for i, e in enumerate(node.arguments):
            e.accept(self)

            if i != len(node.arguments) - 1:
                self._writer.write(", ")

        if node.return_values:
            for e in node.return_values:
                self._writer.write(", ")
                e.type.accept(self)
                self._writer.write("* ")
                e.name.accept(self)

        self._writer.write_line(")")
        self._writer.write_line("{")
        self._writer.indent()

        # Code generation for sending
        self._writer.write_line(f"// Opcode '{node.name.value}' = {self._current_opcode};")
        self._writer.write_line("rpc::Message message;")
        self._writer.write_line("message.source = id();")
        self._writer.write_line("message.destination = remote_id();")
        self._writer.write_line(f"message.opcode = {self._current_opcode};")
        self._writer.write_line("rpc::Serializer s;")

        # Serializing the arguments
        for e in node.arguments:
            self._writer.write("s.serialize(")
            e.name.accept(self)
            self._writer.write_line(");")

        self._writer.write_line("message.payload = s.get();")

        if node.return_values is None:
            self._writer.write_line("return channel_->send_message(remote_port(), message);")
        else:
            # Handling the return values
            self._writer.write_line("rpc::Message result;")
            self._writer.write_line("if (!channel_->send_request(remote_port(), message, result))")
            self._writer.indent()
            self._writer.write_line("return false;")
            self._writer.deindent()

            self._writer.write_line("rpc::Unserializer u(std::move(result.payload));")

            for e in node.return_values:
                self._writer.write("if (!u.unserialize(")
                e.name.accept(self)
                self._writer.write_line(")")

                self._writer.indent()
                self._writer.write_line("return false;")
                self._writer.deindent()

            self._writer.write_line("return true;")

        self._writer.deindent()
        self._writer.write_line("}")

    def visit_Interface(self, node: Interface) -> None:
        self._current_interface = node.name.value
        self._current_opcode = 0

        for method in node.methods:
            method.accept(self)
            self._current_opcode += 1

    def visit_Namespace(self, node: Namespace) -> None:
        self._writer.write_line(f"namespace {node.name.value}")
        self._writer.write_line("{")
        self._writer.indent()

        for elem in node.elements:
            elem.accept(self)

        self._writer.deindent()
        self._writer.write_line("}")

    @property
    def data(self) -> str:
        return self._writer.data()
