#ifndef RPC_BROKER_HH
#define RPC_BROKER_HH

#include <unordered_map>

#include "protoipc/port.hh"
#include "protorpc/rpcobject.hh"

namespace rpc
{
    using PortId = std::uint64_t;
    using ObjectId = std::uint64_t;

    class Broker
    {
    public:
        Broker();

        /**
         * Add new port to listen on for events and returns its id in the
         * broker.
         */
        PortId add_port(ipc::Port port);

        /**
         * Stop listening on a given port.
         */
        void remove_port(PortId port_id);

        /**
         * Handle incomming events.
         */
        void loop();

    private:
        /**
         * Reads a message from the group of ports.
         * Returns the message and the port id from which the message was read.
         */
        ipc::PortError next_message_(ipc::Message& out_message, PortId& out_port);

        std::unordered_map<PortId, ipc::Port> ports_;

        /**
         * Maps object_id to port_id.
         */
        std::unordered_map<ObjectId, PortId> object_mappings_;

#ifdef __linux__
        int epoll_fd_ = -1;
        PortId port_id_ = 0;
#else
        #error "Unsupported platform"
#endif
    };

}

#endif
