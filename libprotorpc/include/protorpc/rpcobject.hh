#ifndef RPC_RPCOBJECT_HH
#define RPC_RPCOBJECT_HH

#include <cstdint>
#include <memory>

namespace rpc
{
    class Channel;
    class Message;

    class RpcObject
    {
    public:
        RpcObject(Channel* chan, std::uint64_t id);

        std::uint64_t id() const
        {
            return object_id_;
        }

    protected:
        Channel* channel_;

    private:
        std::uint64_t object_id_;
    };

    class RpcProxy : public RpcObject
    {
    public:
        RpcProxy(Channel* chan, std::uint64_t id, std::uint64_t remote_port, std::uint64_t remote_id);

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

    class RpcReceiver : public RpcObject
    {
    public:
        RpcReceiver(Channel* chan, std::uint64_t id);
        virtual ~RpcReceiver() {};

        /**
         * Handler called by the channel when a message received for the given
         * receiver.
         */
        virtual void on_message(std::uint64_t source_port, rpc::Message& message) = 0;
    };

    template <typename T>
    using Receiver = std::shared_ptr<std::enable_if_t<std::is_base_of_v<RpcReceiver, T>, T>>;

    template <typename T>
    using Proxy = std::shared_ptr<std::enable_if_t<std::is_base_of_v<RpcProxy, T>, T>>;
}

#endif
