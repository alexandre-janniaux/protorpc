#include <stdexcept>
#include "protorpc/channel.hh"
#include "protorpc/broker.hh"

namespace rpc
{

Channel::Channel(ipc::Port port, std::uint64_t broker_proxy_id)
    : port_(port)
{
    broker_ = std::make_shared<BrokerProxy>(this, broker_proxy_id, 0);
}

void Channel::send_message(ipc::Message& message)
{
    port_.send(message);
}

ipc::Message Channel::send_request(ipc::Message& message, std::uint64_t sender)
{
    send_message(message);

    // Now we wait synchronously for an answer, if the message is not for us
    // we add it to the message queue.
    for (;;)
    {
        ipc::Message cur_msg;
        ipc::PortError err = port_.receive(cur_msg);

        // TODO: Find correct way of handling errors.
        if (err != ipc::PortError::Ok)
            throw std::runtime_error("Error while reading from port");

        if (cur_msg.destination == sender)
        {
            if (cur_msg.opcode != message.opcode)
                throw std::runtime_error("Reply opcode doesn't match");

            return cur_msg;
        }

        message_queue_.push_back(std::move(cur_msg));
    }
}

void Channel::loop()
{
    for (;;)
    {
        ipc::Message msg;
        ipc::PortError err = port_.receive(msg);

        if (err != ipc::PortError::Ok)
            throw std::runtime_error("Error while reading from port");

        // XXX: Handle transmission errors
        message_queue_.push_back(std::move(msg));

        while (!message_queue_.empty())
        {
            ipc::Message cur_msg = std::move(message_queue_.front());
            message_queue_.pop_front();

            auto handler = receivers_.find(cur_msg.destination);

            if (handler == receivers_.end())
                throw std::runtime_error("Not object bound for requested id");

            auto& [id, object] = *handler;
            object->on_message(cur_msg);
        }
    }
}

void Channel::bind_object(std::uint64_t object_id)
{
    ipc::Message msg;
    msg.destination = 0; // Broker is always id 0
    msg.opcode = 0; // BIND_OBJECT opcode

    std::uint8_t* start = reinterpret_cast<std::uint8_t*>(&object_id);
    msg.payload.insert(msg.payload.begin(), start, start + sizeof(object_id));
}

}
