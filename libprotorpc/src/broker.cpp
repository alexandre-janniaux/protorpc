#include "protorpc/broker.hh"

namespace rpc
{

BrokerReceiver::BrokerReceiver(Channel* chan, Broker* broker, std::uint64_t id, std::uint64_t remote)
    : RpcReceiver(chan, id, remote), broker_(broker)
{}

BrokerProxy::BrokerProxy(Channel* chan, std::uint64_t id, std::uint64_t remote)
    : RpcProxy(chan, id, remote)
{}

}
