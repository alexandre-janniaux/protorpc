#include "protorpc/rpcobject.hh"
#include "protorpc/channel.hh"

namespace rpc
{

RpcObject::RpcObject(Channel* chan, std::uint64_t id, std::uint64_t remote)
    : channel_(chan), object_id_(id), remote_id_(remote)
{}

void RpcObject::send_message(ipc::Message& msg) const
{
    channel_->send_message(msg);
}

ipc::Message RpcProxy::send_request(ipc::Message& msg) const
{
    return channel_->send_request(msg, id());
}


RpcProxy::RpcProxy(Channel* chan, std::uint64_t id, std::uint64_t remote)
    : RpcObject(chan, id, remote)
{}

RpcReceiver::RpcReceiver(Channel* chan, std::uint64_t id, std::uint64_t remote)
    : RpcObject(chan, id, remote)
{}

}
