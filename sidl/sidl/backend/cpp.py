from typing import Dict, List
from sidl.utils import IndentedWriter
from sidl.ast import Visitor, Symbol, Method, Type, Interface, Namespace, VariableDeclaration, Struct, AstNode


class BaseCppCompiler(Visitor):
    """
    Common code shared between all compiler visitors
    """

    writer: IndentedWriter
    _types: Dict[str, str]

    def __init__(self, types: Dict[str, str], indent: int = 4) -> None:
        self._types = types
        self.writer = IndentedWriter(indent)

    def visit_Type(self, node: Type) -> None:
        """
        Converts a type to its C++ representation. We assume that the ast passed
        the type checker checks and that everything is valid. If in any case a
        type doesn't exist in the provided type checker mapping we let it as is.
        """
        cpp_type = self._types.get(node.value, node.value)
        self.writer.write(cpp_type)

        if len(node.generics) > 0:
            self.writer.write("<")

            for i, e in enumerate(node.generics):
                e.accept(self)

                if i != len(node.generics) - 1:
                    self.writer.write(", ")

            self.writer.write(">")

    def visit_Symbol(self, node: Symbol) -> None:
        self.writer.write(node.value)

    def visit_VariableDeclaration(self, decl: VariableDeclaration) -> None:
        decl.type.accept(self)
        self.writer.write(" ")
        decl.name.accept(self)

    def visit_Namespace(self, node: Namespace) -> None:
        self.writer.write_line(f"namespace {node.name.value}")
        self.writer.write_line("{")
        self.writer.indent()

        for elem in node.elements:
            elem.accept(self)

        self.writer.deindent()
        self.writer.write_line("}")


class SourceCompiler(BaseCppCompiler):
    writer: IndentedWriter
    _current_interface: str
    _current_opcode: int
    _types: Dict[str, str]
    _filename: str

    def __init__(self, filename: str, types: Dict[str, str], indent: int = 4) -> None:
        super().__init__(types, indent)
        self._current_opcode = 0
        self._types = types
        self._filename = filename

    def visit_Struct(self, node: Struct) -> None:
        pass

    def _compile_proxy_method(self, node: Method) -> None:
        # prototype generation
        self.writer.write(f"bool {self._current_interface}Proxy::{node.name.value}(")

        for i, e in enumerate(node.arguments):
            e.type.accept(self)
            self.writer.write(" ")
            self.writer.write("__sidl_argument_")
            e.name.accept(self)

            if i != len(node.arguments) - 1:
                self.writer.write(", ")

        if node.return_values:
            for i, e in enumerate(node.return_values):
                if len(node.arguments) > 0 or i > 0:
                    self.writer.write(", ")

                e.type.accept(self)
                self.writer.write("* ")
                self.writer.write("__sidl_retval_")
                e.name.accept(self)

        self.writer.write_line(")")
        self.writer.write_line("{")
        self.writer.indent()

        # Code generation for sending
        self.writer.write_line(f"// Opcode '{node.name.value}' = {self._current_opcode};")
        self.writer.write_line("rpc::Message __sidl_message;")
        self.writer.write_line("__sidl_message.source = id();")
        self.writer.write_line("__sidl_message.destination = remote_id();")
        self.writer.write_line(f"__sidl_message.opcode = {self._current_opcode};")
        self.writer.write_line("rpc::Serializer __sidl_s;")

        # Serializing the arguments
        for e in node.arguments:
            if e.type.value == "handle":
                self.writer.write("__sidl_s.add_handle(__sidl_argument_")
                e.name.accept(self)
                self.writer.write_line(");")
            else:
                self.writer.write("__sidl_s.serialize(__sidl_argument_")
                e.name.accept(self)
                self.writer.write_line(");")

        self.writer.write_line("__sidl_message.payload = __sidl_s.get_payload();")
        self.writer.write_line("__sidl_message.handles = __sidl_s.get_handles();")

        if node.return_values is None:
            self.writer.write_line("return channel_->send_message(remote_port(), __sidl_message);")
        else:
            # Handling the return values
            self.writer.write_line("rpc::Message __sidl_result;")
            self.writer.write_line("if (!channel_->send_request(remote_port(), __sidl_message, __sidl_result))")
            self.writer.indent()
            self.writer.write_line("return false;")
            self.writer.deindent()

            self.writer.write_line("rpc::Unserializer __sidl_u(std::move(__sidl_result.payload), __sidl_result.handles);")

            for e in node.return_values:
                if e.type.value == "handle":
                    self.writer.write("if (!__sidl_u.next_handle(__sidl_retval_")
                    e.name.accept(self)
                    self.writer.write_line(")")

                    self.writer.indent()
                    self.writer.write_line("return false;")
                    self.writer.deindent()
                else:
                    self.writer.write("if (!__sidl_u.unserialize(__sidl_retval_")
                    e.name.accept(self)
                    self.writer.write_line("))")

                    self.writer.indent()
                    self.writer.write_line("return false;")
                    self.writer.deindent()

            self.writer.write_line("return true;")

        self.writer.deindent()
        self.writer.write_line("}")

    def _compile_receiver_method(self, node: Method) -> None:
        # Step 1: Deserialize arguments
        self.writer.write_line("rpc::Unserializer __sidl_u(std::move(__sidl_message.payload), __sidl_message.handles);")
        call_stmt = f"{node.name.value}("

        for i, e in enumerate(node.arguments):
            e.type.accept(self)
            self.writer.write(" __sidl_argument_")
            e.name.accept(self)
            self.writer.write_line(";")

            arg_type = e.type.value
            arg_name = e.name.value

            if i > 0:
                call_stmt += ", "

            call_stmt += f"__sidl_argument_{arg_name}"

            if arg_type == "handle":
                self.writer.write_line(f"if (!__sidl_u.next_handle(&__sidl_argument_{arg_name}))")
                self.writer.indent()
                self.writer.write_line("return; // TODO: Maybe return an error code ?")
                self.writer.deindent()
            else:
                self.writer.write_line(f"if (!__sidl_u.unserialize(&__sidl_argument_{arg_name}))")
                self.writer.indent()
                self.writer.write_line("return; // TODO: Maybe return an error code ?")
                self.writer.deindent()

        if node.return_values:
            for i, e in enumerate(node.return_values):
                e.type.accept(self)
                self.writer.write(" __sidl_retval_")
                e.name.accept(self)
                self.writer.write_line(";")

                if len(node.arguments) > 0 or i > 0:
                    call_stmt += ", "

                call_stmt += f"&__sidl_retval_{e.name.value}"

        call_stmt += ");"

        # Step 2: Call handler function
        self.writer.write_line(call_stmt)

        # Step 3: Send back answer if needed
        if node.return_values is None:
            return

        self.writer.write_line("rpc::Message __sidl_reply;")
        self.writer.write_line("__sidl_reply.source = __sidl_id;")
        self.writer.write_line("__sidl_reply.destination = __sidl_message.destination;")
        self.writer.write_line("__sidl_reply.opcode = __sidl_message.opcode;")
        self.writer.write_line("rpc::Serializer __sidl_s;")

        for e in node.return_values:
            ret_type = e.type.value
            ret_name = e.name.value

            if ret_type == "handle":
                self.writer.write_line(f"__sidl_s.add_handle(__sidl_retval_{ret_name});")
            else:
                self.writer.write_line(f"__sidl_s.serialize(__sidl_retval_{ret_name});")

        self.writer.write_line("__sidl_reply.payload = __sidl_s.get_payload();")
        self.writer.write_line("__sidl_reply.handles = __sidl_s.get_handles();")
        self.writer.write_line("__sidl_channel.send_message(__sidl_source_port, __sidl_reply);")

    def _compile_proxy_interface(self, node: Interface) -> None:
        self._current_interface = node.name.value
        self._current_opcode = 0

        for method in node.methods:
            self._compile_proxy_method(method)
            self._current_opcode += 1

    def _compile_receiver_interface(self, node: Interface) -> None:
        self._current_interface = node.name.value
        self._current_opcode = 0

        self.writer.write_line(f"void {node.name.value}Receiver::on_message(rpc::Channel& __sidl_channel, rpc::ObjectId __sidl_id, rpc::PortId __sidl_source_port, rpc::Message& __sidl_message)")
        self.writer.write_line("{")
        self.writer.indent()
        self.writer.write_line("switch (__sidl_message.opcode)")
        self.writer.write_line("{")

        for method in node.methods:
            method_name = method.name.value
            self.writer.write_line(f"case {self._current_opcode}: // Opcode '{method_name}' = {self._current_opcode}")
            self.writer.write_line("{")

            self.writer.indent()
            self._compile_receiver_method(method)
            self.writer.write_line("break;")
            self.writer.deindent()

            self.writer.write_line("}")

            self._current_opcode += 1

        self.writer.write_line("default:")
        self.writer.indent()
        self.writer.write_line("return; // Return error code maybe ?")
        self.writer.deindent()

        self.writer.write_line("}")
        self.writer.deindent()
        self.writer.write_line("}")

    def visit_Interface(self, node: Interface) -> None:
        self._compile_proxy_interface(node)
        self._compile_receiver_interface(node)

    def visit(self, node: AstNode) -> None:
        self.writer.write_line(f"#include \"{self._filename}.hh\"")
        self.writer.write_line("")

        node.accept(self)

    @property
    def data(self) -> str:
        return self.writer.data()


class PendingStruct:
    namespace: List[str]
    struct: Struct

    def __init__(self, namespace: List[str], struct: Struct) -> None:
        self.namespace = namespace
        self.struct = struct


class HeaderCompiler(BaseCppCompiler):
    _structs: List[PendingStruct]
    _namespace: List[str]
    _filename: str

    def __init__(self, filename: str, types: Dict[str, str], indent: int = 4) -> None:
        super().__init__(types, indent)
        self._structs = []
        self._namespace = []
        self._filename = filename

    def _compile_proxy_method(self, node: Method) -> None:
        name = node.name.value

        self.writer.write(f"bool {name}(")

        for i, e in enumerate(node.arguments):
            e.accept(self)

            if i != len(node.arguments) - 1:
                self.writer.write(", ")

        if node.return_values:
            for i, e in enumerate(node.return_values):
                if len(node.arguments) > 0 or i > 0:
                    self.writer.write(", ")

                e.type.accept(self)
                self.writer.write(f"* {e.name.value}")

        self.writer.write_line(");")

    def _compile_receiver_method(self, node: Method) -> None:
        name = node.name.value

        self.writer.write(f"virtual bool {name}(")

        for i, e in enumerate(node.arguments):
            e.accept(self)

            if i != len(node.arguments) - 1:
                self.writer.write(", ")

        if node.return_values:
            for i, e in enumerate(node.return_values):
                if len(node.arguments) > 0 or i > 0:
                    self.writer.write(", ")

                e.type.accept(self)
                self.writer.write(f"* {e.name.value}")

        # XXX: Hack
        self.writer.write_line(") = 0;")

    def _compile_proxy_interface(self, node: Interface) -> None:
        interface_name = node.name.value
        self.writer.write_line(f"class {interface_name}Proxy: public rpc::RpcProxy")
        self.writer.write_line("{")
        self.writer.write_line("public:")
        self.writer.indent()

        # Constructor
        self.writer.write_line(f"{interface_name}Proxy(rpc::Channel* chan, std::uint64_t object_id, std::uint64_t remote_port, std::uint64_t remote_id)")
        self.writer.indent()
        self.writer.write_line(": rpc::RpcProxy(chan, object_id, remote_port, remote_id)")
        self.writer.deindent()
        self.writer.write_line("{}")

        # Destructor
        self.writer.write_line(f"virtual ~{interface_name}Proxy() {{}}")

        # Method declaration
        for method in node.methods:
            self._compile_proxy_method(method)

        self.writer.deindent()
        self.writer.write_line("};")

    def _compile_receiver_interface(self, node: Interface) -> None:
        interface_name = node.name.value
        self.writer.write_line(f"class {interface_name}Receiver: public rpc::RpcReceiver")
        self.writer.write_line("{")
        self.writer.write_line("public:")
        self.writer.indent()

        # Destructor
        self.writer.write_line(f"virtual ~{interface_name}Receiver() {{}}")

        # on_message overload
        self.writer.write_line("void on_message(rpc::Channel& channel, rpc::ObjectId object, rpc::PortId source_port, rpc::Message& message) override;")

        for method in node.methods:
            self._compile_receiver_method(method)

        self.writer.deindent()
        self.writer.write_line("};")

    def visit_Interface(self, node: Interface) -> None:
        self._compile_proxy_interface(node)
        self._compile_receiver_interface(node)

    def _compile_struct_decl(self, node: Struct) -> None:
        struct_name = node.name.value

        self.writer.write_line(f"struct {struct_name}")
        self.writer.write_line("{")
        self.writer.indent()

        for field in node.fields:
            field.accept(self)
            self.writer.write_line(";")

        self.writer.deindent()
        self.writer.write_line("};")

    def _compile_struct_serialize(self, p: PendingStruct) -> None:
        struct_type = p.struct.name.value

        if len(p.namespace) > 0:
            struct_type = "::".join(p.namespace) + "::" + struct_type

        self.writer.write_line("namespace rpc")
        self.writer.write_line("{")
        self.writer.indent()

        self.writer.write_line("template <>")
        self.writer.write_line(f"struct serializable<{struct_type}>")
        self.writer.write_line("{")
        self.writer.indent()

        self.writer.write_line(f"static void serialize({struct_type}& __sidl_obj, Serializer& __sidl_s)")
        self.writer.write_line("{")
        self.writer.indent()

        for field in p.struct.fields:
            ty_name = field.type.value
            field_name = field.name.value

            if ty_name == "handle":
                self.writer.write_line(f"__sidl_s.add_handle(__sidl_obj.{field_name});")
            else:
                self.writer.write_line(f"__sidl_s.serialize(__sidl_obj.{field_name});")

        self.writer.deindent()
        self.writer.write_line("}")

        self.writer.deindent()
        self.writer.write_line("};")

        self.writer.deindent()
        self.writer.write_line("}")

    def _compile_struct_unserialize(self, p: PendingStruct) -> None:
        struct_type = p.struct.name.value

        if len(p.namespace) > 0:
            struct_type = "::".join(p.namespace) + "::" + struct_type

        self.writer.write_line("namespace rpc")
        self.writer.write_line("{")
        self.writer.indent()

        self.writer.write_line("template <>")
        self.writer.write_line(f"struct unserializable<{struct_type}>")
        self.writer.write_line("{")
        self.writer.indent()

        self.writer.write_line(f"static bool unserialize({struct_type}* __sidl_obj, Unserializer& __sidl_u)")
        self.writer.write_line("{")
        self.writer.indent()

        for field in p.struct.fields:
            ty_name = field.type.value
            field_name = field.name.value

            if ty_name == "handle":
                self.writer.write_line(f"if (!__sidl_u.next_handle(&__sidl_obj->{field_name}))")
            else:
                self.writer.write_line(f"if (!__sidl_u.unserialize(&__sidl_obj->{field_name}))")

            self.writer.indent()
            self.writer.write_line("return false;")
            self.writer.deindent()

        self.writer.write_line("return true;")

        self.writer.deindent()
        self.writer.write_line("}")

        self.writer.deindent()
        self.writer.write_line("};")

        self.writer.deindent()
        self.writer.write_line("}")

    def visit_Struct(self, node: Struct) -> None:
        p = PendingStruct([s for s in self._namespace], node)
        self._structs.append(p)
        self._compile_struct_decl(node)

    def visit_Namespace(self, node: Namespace) -> None:
        name = node.name.value
        self._namespace.append(name)

        self.writer.write_line(f"namespace {name}")
        self.writer.write_line("{")
        self.writer.indent()

        for elem in node.elements:
            elem.accept(self)

        self.writer.deindent()
        self.writer.write_line("}")

        self._namespace.pop()

    def visit(self, root: AstNode) -> None:
        # Mandatory includes
        header_name = self._filename.replace(".", "_").upper() + "_HH"
        self.writer.write_line(f"#ifndef {header_name}")
        self.writer.write_line(f"#define {header_name}")
        self.writer.write_line("#include \"protorpc/serializer.hh\"")
        self.writer.write_line("#include \"protorpc/unserializer.hh\"")
        self.writer.write_line("#include \"protorpc/rpcobject.hh\"")
        self.writer.write_line("#include \"protorpc/message.hh\"")
        self.writer.write_line("#include \"protorpc/channel.hh\"")
        self.writer.write_line("")

        # Generate code from namespace
        root.accept(self)

        # Generate out of namespace template specialization code
        for p in self._structs:
            self._compile_struct_unserialize(p)
            self._compile_struct_serialize(p)

        self.writer.write_line("")
        self.writer.write_line("#endif")

    @property
    def data(self) -> str:
        return self.writer.data()
