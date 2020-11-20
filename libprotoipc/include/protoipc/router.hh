#ifndef IPC_ROUTER_HH
#define IPC_ROUTER_HH

#include <unordered_map>
#include "protoipc/port.hh"

namespace ipc
{
    using PortId = std::uint64_t;

    /**
     * Central node of the ipc layer. It routes ipc::Messages between multiple
     * ipc::Ports.
     */
    class Router
    {
    public:
        Router();
        ~Router();

        /**
         * Adds a new port to listen on. Returns the PortId associated with the
         * port object.
         */
        PortId add_port(ipc::Port port);

        /**
         * Removes a destination port from the router. Returns true if port could
         * be removed.
         */
        bool remove_port(PortId id);

        /**
         * Handles requests and routes messages. Messages with unknown desintation
         * are dropped.
         */
        ipc::PortError loop();

    private:
        // XXX: Should this be protected by a mutex ?
        std::unordered_map<PortId, Port> ports_;
        PortId current_id_ = 0;

#ifdef __linux__
        int epoll_fd_ = -1;
#else
#error "Unsupported platform for ipc"
#endif
    };
}

#endif
