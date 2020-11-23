#ifndef RPC_EXCHANNEL_HH
#define RPC_EXCHANNEL_HH

#include <memory>
#include <deque>
#include <unordered_map>
#include <unordered_set>
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
         * Binds a receiver to the current channel.
         */
        template <typename T>
        ExReceiver<T> bind()
        {
            ExReceiver<T> object = std::make_shared<T>(this, current_id_++);
            receivers_[object->id()] = object;

            return object;
        }

        /**
         * Binds a proxy to the current channel.
         */
        template <typename T>
        ExProxy<T> bind(std::uint64_t remote_port, std::uint64_t remote_id)
        {
            ExProxy<T> object = std::make_shared<T>(this, current_id_++, remote_port, remote_id);
            return object;
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
