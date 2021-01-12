#include <stdexcept>
#include <sys/epoll.h>
#include <unistd.h>
#include "protoipc/router.hh"

namespace ipc
{

Router::Router()
{
    epoll_fd_ = epoll_create1(0);

    if (epoll_fd_ == -1)
        throw std::runtime_error("Could not create epoll fd");
}

Router::~Router()
{
    close(epoll_fd_);
}

PortId Router::add_port(ipc::Port port)
{
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.u64 = current_id_;

    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, port.handle(), &ev) == -1)
    {
        std::perror("EPOLL_CTL_ADD");
        throw std::runtime_error("epoll");
    }

    ports_[current_id_] = port;

    return current_id_++;
}

bool Router::remove_port(PortId id)
{
    auto it = ports_.find(id);

    if (it == ports_.end())
        return false;

    ipc::Port port_obj = it->second;
    ports_.erase(it);

    // XXX: Should we close the port or leave this to the caller who added it ?
    port_obj.close();

    return true;
}

ipc::PortError Router::loop()
{
    for (;;)
    {
        constexpr int EPOLL_MAX_EVENTS = 16;
        struct epoll_event events[EPOLL_MAX_EVENTS];

        int res = 0;

        while ((res = epoll_wait(epoll_fd_, events, EPOLL_MAX_EVENTS, -1)) == -1)
        {
            if (errno == EINTR)
                continue;
            else
                return ipc::PortError::PollError;
        }

        for (int i = 0; i < res; i++)
        {
            struct epoll_event ev = events[i];
            auto source = ports_.find(ev.data.u64);

            if (source == ports_.end())
                return ipc::PortError::BadFileDescriptor;

            ipc::Message message;
            ipc::PortError err = source->second.receive(message);

            if (err != ipc::PortError::Ok)
                return err;

            auto destination = ports_.find(message.destination);

            if (destination == ports_.end())
                return ipc::PortError::BadFileDescriptor;

            // We patch the message and replace the destination's process id by the
            // sender's process id. The receiver can then know to who reply.
            message.destination = source->first;
            err = destination->second.send(message);

            if (err != ipc::PortError::Ok)
                return err;
        }
    }
}

}
