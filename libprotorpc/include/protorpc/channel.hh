#ifndef RPC_CHANNEL_HH
#define RPC_CHANNEL_HH

#include <unordered_map>
#include <deque>

#include "protoipc/port.hh"
#include "protorpc/rpcobject.hh"

namespace rpc
{

    class Channel
    {
    public:
        Channel(ipc::Port port);

        /**
         * Binds a receiver to the current channel and returns a reference to it.
         * Notifies the broker through the port of the new object's creation.
         */
        template <typename T>
        Receiver<T> bind(std::uint64_t object_id, std::uint64_t remote)
        {
            Receiver<T> object = std::make_shared<T>(this, object_id, remote);
            receivers_.emplace(object_id, object);
            bind_object(object_id);

            return object;
        }

        /**
         * Binds a proxy to the current channel and returns a reference to it.
         * Notifies the broker through the port of the new object's creation.
         */
        template <typename T>
        Proxy<T> bind(std::uint64_t object_id, std::uint64_t remote)
        {
            Proxy<T> object = std::make_shared<T>(this, object_id, remote);
            bind_object(object_id);

            return object;
        }

        /**
         * Sends a message through the native socket.
         */
        void send_message(ipc::Message& message);

        /**
         * Sends a request and synchronously waits for an answer).
         * XXX: Is returning ipc::Message instead of error code a good idea ?
         */
        ipc::Message send_request(ipc::Message& message, std::uint64_t sender);

        /**
         * Main event loop.
         */
        void loop();

    private:
        /**
         * Notifies the broker of the binding of a local object.
         * This binding is necessary for message routing.
         */
        void bind_object(std::uint64_t object_id);

        ipc::Port port_;
        std::unordered_map<std::uint64_t, std::shared_ptr<RpcReceiver>> receivers_;

        // XXX: Do we need to be threadsafe if the typical use case is
        //      single-threaded ?
        std::deque<ipc::Message> message_queue_;
    };

}

#endif
