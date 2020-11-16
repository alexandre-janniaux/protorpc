#ifndef RPC_BROKER_HH
#define RPC_BROKER_HH

#include "protoipc/port.hh"
#include "protorpc/rpcobject.hh"

namespace rpc
{
    class Broker
    {
    public:
        Broker();

    private:
        ipc::Port port_;
    };

    class BrokerProxy : public RpcProxy
    {
    public:
        BrokerProxy(Channel* chan, std::uint64_t id, std::uint64_t remote);

        void register_id(std::uint64_t id);
    };

    class BrokerReceiver : public RpcReceiver
    {
    public:
        BrokerReceiver(Channel* chan, Broker* broker, std::uint64_t id, std::uint64_t remote);

        virtual void on_message(ipc::Message& message) override
        {};

    private:
        Broker* broker_;
    };
}

#endif
