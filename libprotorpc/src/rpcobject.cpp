#include "protorpc/rpcobject.hh"

namespace rpc
{

RpcObject::RpcObject(Channel* chan, std::uint64_t id, std::uint64_t remote)
    : channel_(chan), object_id_(id), remote_id_(remote)
{}

RpcProxy::RpcProxy(Channel* chan, std::uint64_t id, std::uint64_t remote)
    : RpcObject(chan, id, remote)
{}

RpcReceiver::RpcReceiver(Channel* chan, std::uint64_t id, std::uint64_t remote)
    : RpcObject(chan, id, remote)
{}

}
