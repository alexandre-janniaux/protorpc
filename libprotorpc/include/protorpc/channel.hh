#ifndef RPC_CHANNEL_HH
#define RPC_CHANNEL_HH

#include <memory>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include "protoipc/port.hh"
#include "protorpc/message.hh"
#include "protorpc/rpcobject.hh"

namespace rpc
{
    class Channel
    {
    public:
        struct PendingRpcMessage
        {
            PortId source_port;
            ObjectId destination_object;
            rpc::Message message;
        };

        Channel(PortId port_id, ipc::Port port);

        /**
         * Binds a receiver to the current channel and return its id.
         */
        template <typename T, typename... Ts>
        ObjectId bind(Ts&&... args)
        {
            next_id_();
            Receiver<T> object = std::make_shared<T>(std::forward<Ts>(args)...);
            receivers_[current_id_] = object;
            allocated_objects_.emplace(current_id_);

            return current_id_;
        }

        /**
         * Binds a receiver to the current channel with a specific id, overwriting the previous
         * object if it exists.
         */
        template <typename T, typename... Ts>
        void bind_static(ObjectId id, Ts&&... args)
        {
            Receiver<T> object = std::make_shared<T>(std::forward<Ts>(args)...);
            receivers_[id] = object;
            allocated_objects_.emplace(id);
        }

        /**
         * Binds a proxy to the current channel.
         */
        template <typename T>
        Proxy<T> connect(PortId remote_port, ObjectId remote_id)
        {
            next_id_();
            Proxy<T> object = std::make_shared<T>(this, current_id_, remote_port, remote_id);
            allocated_objects_.emplace(current_id_);
            return object;
        }

        /**
         * Handles the event loop.
         */
        void loop();

        /**
         * Send an unidirectional message to a remote object.
         */
        bool send_message(PortId remote_port, rpc::Message& msg);

        /**
         * Sends a bidirectional message to a remote object. Blocks the event loop while
         * waiting for an answer.
         */
        bool send_request(PortId remote_port, rpc::Message& msg, rpc::Message& result);

    private:
        /**
         * Advance to the next available id.
         */
        void next_id_();

        PortId port_id_;
        ipc::Port port_;
        ObjectId current_id_ = 0;

        std::unordered_map<ObjectId, std::shared_ptr<RpcReceiver>> receivers_;

        /**
         * Keeps the id of all allocated objects. Needed for computing the next available
         * id.
         */
        std::unordered_set<ObjectId> allocated_objects_;

        /**
         * Extracts the payload from the ipc message and does preprocessing on the
         * rpc message (swapping source and destination object ids).
         */
        PendingRpcMessage next_message_();

        std::deque<PendingRpcMessage> message_queue_;
    };
}

#endif
