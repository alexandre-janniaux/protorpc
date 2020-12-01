#include "protorpc/rpcobject.hh"

namespace rpc
{

class Channel;

RpcProxy::RpcProxy(Channel* chan, std::uint64_t id, std::uint64_t remote_port, std::uint64_t remote_id)
    : channel_(chan), id_(id), remote_port_(remote_port), remote_id_(remote_id)
{}

}
