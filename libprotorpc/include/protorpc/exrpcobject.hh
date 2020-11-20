#ifndef RPC_EXRPCOBJECT_HH
#define RPC_EXRPCOBJECT_HH

#include <cstdint>
#include <memory>
#include "protorpc/message.hh"

namespace rpc
{
    class Channel;

    class ExRpcObject
    {
    public:
        ExRpcObject(Channel* chan, std::uint64_t id);

        const std::uint64_t id()
        {
            return object_id_;
        }

    protected:
        Channel* channel_;

    private:
        std::uint64_t object_id_;
    };

    class ExRpcProxy : public ExRpcObject
    {
    public:
        ExRpcProxy(Channel* chan, std::uint64_t id, std::uint64_t remote_port, std::uint64_t remote_id);

        std::uint64_t remote_port() const
        {
            return remote_port_;
        }

        std::uint64_t remote_id() const
        {
            return remote_id_;
        }

    private:
        std::uint64_t remote_port_;
        std::uint64_t remote_id_;
    };

    class ExRpcReceiver : public ExRpcObject
    {
    public:
        ExRpcReceiver(Channel* chan, std::uint64_t id);
        virtual ~ExRpcReceiver();

        /**
         * Handler called by the channel when a message received for the given
         * receiver.
         */
        virtual void on_message(std::uint64_t source_port, rpc::Message& message) = 0;
    };

    template <typename T>
    using Receiver = std::shared_ptr<std::enable_if_t<std::is_base_of_v<ExRpcReceiver, T>, T>>;

    template <typename T>
    using Proxy = std::shared_ptr<std::enable_if_t<std::is_base_of_v<ExRpcProxy, T>, T>>;
}

#endif
