#include <vector>
#include <cstdio>
#include <stdexcept>
#include <sys/epoll.h>
#include "fmt/core.h"
#include "protorpc/broker.hh"
#include "protorpc/unserializer.hh"

namespace rpc
{

Broker::Broker()
{
    epoll_fd_ = epoll_create1(0);

    if (epoll_fd_ == -1)
        throw std::runtime_error("Could not create epoll fd");
}

PortId Broker::add_port(ipc::Port port)
{
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.u64 = port_id_;

    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, port.handle(), &ev) == -1)
    {
        std::perror("EPOLL_CTL_ADD");
        throw std::runtime_error("epoll failed");
    }

    ports_[port_id_] = port;

    return port_id_++;
}

void Broker::remove_port(PortId port)
{
    auto it = ports_.find(port);

    // XXX : Maybe return an error instead ?
    if (it == ports_.end())
        throw std::runtime_error("Could not find port to remove");

    ipc::Port port_obj = it->second;
    ports_.erase(it);

    // Remove object referencing this port.
    for (auto obj_it = object_mappings_.begin(); obj_it != object_mappings_.end(); obj_it++)
    {
        if (obj_it->second == port)
            object_mappings_.erase(obj_it);
    }

    // Close the port
    port_obj.close();
}

ipc::PortError Broker::next_message_(ipc::Message& out_message, PortId& out_port)
{
    constexpr int EPOLL_MAX_EVENTS = 16;
    struct epoll_event events[EPOLL_MAX_EVENTS];

    int res = epoll_wait(epoll_fd_, events, EPOLL_MAX_EVENTS, -1);

    if (res == -1)
        return ipc::PortError::PollError;

    // XXX: This is dumb as we only consider the first event. Adding a message queue
    //      and enqueing would be better but would require more bookkeeping.
    struct epoll_event ev = events[0];
    out_port = ev.data.u64;

    auto it = ports_.find(out_port);

    // XXX: RPC layer errors could be more specific than piggy backing on IPC errors.
    if (it == ports_.end())
        return ipc::PortError::BadFileDescriptor;

    return it->second.receive(out_message);
}

void Broker::loop()
{
    for (;;)
    {
        ipc::Message message;
        PortId port_id;

        if (next_message_(message, port_id) != ipc::PortError::Ok)
            throw std::runtime_error("Error while reading message");

        fmt::print("[BROKER] Received request (destination: {}, opcode: {})\n",
                message.destination, message.opcode);

        // Destination 0 == Broker, Opcode 0 == BIND_OBJECT
        if (message.destination == 0 && message.opcode == 0)
        {
            Unserializer s(std::move(message.payload));
            ObjectId new_object;

            if (!s.unserialize(&new_object))
                throw std::runtime_error("Could not unserialize object id in BIND_OBJECT message");

            object_mappings_.emplace(new_object, port_id);

            fmt::print("[BROKER] Binding object {} from port {}\n", new_object, port_id);
        }
        else
        {
            // Else we forward the message to the good port.
            auto it = object_mappings_.find(message.destination);

            // XXX: Should we just log and drop message on error or throw ?
            if (it == object_mappings_.end())
                throw std::runtime_error("Message destination does not exist");

            ipc::Port target_port = ports_[it->second];

            if (target_port.send(message) != ipc::PortError::Ok)
                throw std::runtime_error("Error while sending message to destination port");

            fmt::print("[BROKER] Forwarding message to port {} fd {}\n", port_id,
                    target_port.handle());
        }

    }
}

}
