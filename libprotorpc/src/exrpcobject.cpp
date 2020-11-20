#include "protorpc/exrpcobject.hh"

namespace rpc
{

class Channel;

ExRpcObject::ExRpcObject(Channel* chan, std::uint64_t id)
    : channel_(chan), object_id_(id)
{}

ExRpcProxy::ExRpcProxy(Channel* chan, std::uint64_t id, std::uint64_t remote_port, std::uint64_t remote_id)
    : ExRpcObject(chan, id), remote_port_(remote_port), remote_id_(remote_id)
{}

}
