#ifndef RPC_RPCOBJECT_HH
#define RPC_RPCOBJECT_HH

#include <cstdint>
#include <memory>
#include <type_traits>
#include "protoipc/message.hh"

namespace rpc
{
    class Channel;

    // TODO: Add specific error types
    enum class RpcError
    {
        Ok = 0,
        Error
    };

    class RpcObject
    {
    public:
        RpcObject(Channel* chan, std::uint64_t id, std::uint64_t remote);

        const std::uint64_t id() const
        {
            return object_id_;
        }

        const std::uint64_t remote() const
        {
            return remote_id_;
        }

        /**
         * Send a message through the bound channel.
         */
        void send_message(ipc::Message& message) const;

    protected:
        Channel* channel_;

    private:
        std::uint64_t object_id_;
        std::uint64_t remote_id_;
    };

    /**
     * Rpc object used to make request and possible receive answers.
     */
    class RpcProxy : public RpcObject
    {
    public:
        RpcProxy(Channel* chan, std::uint64_t id, std::uint64_t remote);

        /**
         * Send a request through the bound channel.
         */
        ipc::Message send_request(ipc::Message& message) const;
    };

    /**
     * Rpc object receiving events.
     */
    class RpcReceiver : public RpcObject
    {
    public:
        RpcReceiver(Channel* chan, std::uint64_t id, std::uint64_t remote);
        virtual ~RpcReceiver() {};

        /**
         * Handler called by the channel when a message is received for the given
         * receiver.
         */
        virtual void on_message(ipc::Message& message) = 0;
    };

    template <typename T>
    using Receiver = std::shared_ptr<std::enable_if_t<std::is_base_of_v<RpcReceiver, T>, T>>;

    template <typename T>
    using Proxy = std::shared_ptr<std::enable_if_t<std::is_base_of_v<RpcProxy, T>, T>>;
}

#endif
