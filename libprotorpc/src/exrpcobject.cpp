#include "protorpc/exrpcobject.hh"

namespace rpc
{

class ExChannel;

ExRpcObject::ExRpcObject(ExChannel* chan, std::uint64_t id)
    : channel_(chan), object_id_(id)
{}

ExRpcProxy::ExRpcProxy(ExChannel* chan, std::uint64_t id, std::uint64_t remote_port, std::uint64_t remote_id)
    : ExRpcObject(chan, id), remote_port_(remote_port), remote_id_(remote_id)
{}

ExRpcReceiver::ExRpcReceiver(ExChannel* chan, std::uint64_t id)
    : ExRpcObject(chan, id)
{}

}
