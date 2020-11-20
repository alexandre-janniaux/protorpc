#ifndef RPC_EXCHANNEL_HH
#define RPC_EXCHANNEL_HH

#include <memory>
#include <deque>
#include <unordered_map>
#include "protoipc/port.hh"
#include "protorpc/message.hh"
#include "protorpc/exrpcobject.hh"

namespace rpc
{
    class ExChannel
    {
    public:
        struct PendingRpcMessage
        {
            std::uint64_t source_port;
            std::uint64_t destination_object;
            rpc::Message message;
        };

        ExChannel(std::uint64_t port_id, ipc::Port port);

        /**
         * Binds a receiver to the current channel using a specific id.
         * XXX: It will overwrite an object if there is an id collision.
         */
        template <typename T>
        Receiver<T> bind(std::uint64_t object_id)
        {
            Receiver<T> object = std::make_shared<T>(this, object_id);
            receivers_[object_id] = object;

            return object;
        }

        /**
         * Binds a proxy to the current channel with a random id.
         */
        template <typename T>
        Receiver<T> bind()
        {
            std::uint64_t id = next_id_();
            return bind<T>(id);
        }

        template <typename T>
        Proxy<T> bind(std::uint64_t object_id, std::uint64_t remote_port, std::uint64_t remote_id)
        {
            Proxy<T> object = std::make_shared<T>(this, object_id, remote_port, remote_id);
            return object;
        }

        template <typename T>
        Proxy<T> bind(std::uint64_t remote_port, std::uint64_t remote_id)
        {
            std::uint64_t id = next_id_();
            return bind<T>(id, remote_port, remote_id);
        }

        /**
         * Handles the event loop.
         */
        void loop();

        /**
         * Send an unidirectional message to a remote object.
         */
        bool send_message(std::uint64_t remote_port, rpc::Message& msg);

        /**
         * Sends a bidirectional message to a remote object. Blocks the event loop while
         * waiting for an answer.
         */
        bool send_request(std::uint64_t remote_port, rpc::Message& msg, rpc::Message& result);

    private:
        // Gets the next available object id
        std::uint64_t next_id_();

        std::uint64_t port_id_;
        ipc::Port port_;
        std::uint64_t current_id_ = 0;

        std::unordered_map<std::uint64_t, std::shared_ptr<ExRpcReceiver>> receivers_;

        /**
         * Extracts the payload from the ipc message and does preprocessing on the
         * rpc message (swapping source and destination object ids).
         */
        PendingRpcMessage next_message_();

        std::deque<PendingRpcMessage> message_queue_;
    };
}

#endif
