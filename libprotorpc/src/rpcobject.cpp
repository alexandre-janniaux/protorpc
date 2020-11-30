#include "protorpc/rpcobject.hh"

namespace rpc
{

class Channel;

RpcObject::RpcObject(Channel* chan, std::uint64_t id)
    : channel_(chan), object_id_(id)
{}

RpcProxy::RpcProxy(Channel* chan, std::uint64_t id, std::uint64_t remote_port, std::uint64_t remote_id)
    : RpcObject(chan, id), remote_port_(remote_port), remote_id_(remote_id)
{}

RpcReceiver::RpcReceiver(Channel* chan, std::uint64_t id)
    : RpcObject(chan, id)
{}

}
