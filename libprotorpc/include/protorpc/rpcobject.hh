#ifndef RPC_RPCOBJECT_HH
#define RPC_RPCOBJECT_HH

#include <cstdint>
#include <memory>

namespace rpc
{
    class Channel;
    class Message;

    class RpcObject
    {};

    using ObjectId = std::uint64_t;
    using PortId = std::uint64_t;

    class RpcProxy : public RpcObject
    {
    public:
        RpcProxy(Channel* chan, ObjectId id, PortId remote_port, ObjectId remote_id)
            : channel_(chan), id_(id), remote_port_(remote_port), remote_id_(remote_id)
        {}

        PortId remote_port() const
        {
            return remote_port_;
        }

        ObjectId remote_id() const
        {
            return remote_id_;
        }

        ObjectId id() const
        {
            return id_;
        }

    protected:
        Channel* channel_;

    private:
        ObjectId id_;
        PortId remote_port_;
        ObjectId remote_id_;
    };

    class RpcReceiver : public RpcObject
    {
    public:
        virtual ~RpcReceiver() {};

        /**
         * Handler called by the channel when a message received for the given
         * receiver.
         */
        virtual void on_message(Channel& chan, ObjectId current_object, PortId source_port, rpc::Message& message) = 0;
    };

    template <typename T>
    using Receiver = std::shared_ptr<std::enable_if_t<std::is_base_of_v<RpcReceiver, T>, T>>;

    template <typename T>
    using Proxy = std::shared_ptr<std::enable_if_t<std::is_base_of_v<RpcProxy, T>, T>>;
}

#endif
