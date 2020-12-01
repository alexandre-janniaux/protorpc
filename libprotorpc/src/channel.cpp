#include <stdexcept>
#include "protorpc/serializer.hh"
#include "protorpc/unserializer.hh"
#include "protorpc/channel.hh"

namespace rpc
{

using PendingRpcMessage = Channel::PendingRpcMessage;

Channel::Channel(std::uint64_t port_id, ipc::Port port)
    : port_id_(port_id), port_(port)
{}

PendingRpcMessage Channel::next_message_()
{
        ipc::Message msg;
        ipc::PortError err = port_.receive(msg);

        if (err != ipc::PortError::Ok)
            throw std::runtime_error("Error while reading from port");

        struct PendingRpcMessage pending;
        pending.source_port = msg.destination;

        // Extract the rpc payload from the message
        rpc::Message result;
        result.handles = std::move(msg.handles);

        Unserializer u(std::move(msg.payload));

        bool status = true;
        std::vector<std::uint8_t> payload;

        status &= u.unserialize(&result.source);
        status &= u.unserialize(&result.destination);
        status &= u.unserialize(&result.opcode);
        status &= u.unserialize(&result.payload);

        if (!status)
            throw std::runtime_error("Could not decode rpc message header");

        // We patch the rpc::Message to indicate the source object.
        pending.destination_object = result.destination;
        result.destination = result.source;
        result.source = pending.destination_object;

        pending.message = std::move(result);

        return pending;
}

void Channel::loop()
{
    for (;;)
    {
        PendingRpcMessage msg = next_message_();
        message_queue_.push_back(std::move(msg));

        while (!message_queue_.empty())
        {
            PendingRpcMessage& pending_msg = message_queue_.front();
            auto handler = receivers_.find(pending_msg.destination_object);

            if (handler == receivers_.end())
                throw std::runtime_error("Destination object not found");

            handler->second->on_message(*this, pending_msg.source_port, pending_msg.message);
            message_queue_.pop_front();
        }
    }
}

bool Channel::send_message(std::uint64_t remote_port, rpc::Message& msg)
{
    ipc::Message ipc_msg;

    // This is the ipc layer, destination is remote process id
    ipc_msg.destination = remote_port;
    ipc_msg.handles = std::move(msg.handles);

    // Encoding the rpc::Message data
    rpc::Serializer s;
    s.serialize(msg.source);
    s.serialize(msg.destination);
    s.serialize(msg.opcode);

    // payload_size is encoded as part of the vector
    s.serialize(msg.payload);

    ipc_msg.payload = s.get_payload();

    ipc::PortError error = port_.send(ipc_msg);

    // TODO: Return a more explicit error than just "failed"
    return error == ipc::PortError::Ok;
}

bool Channel::send_request(std::uint64_t remote_port, rpc::Message& msg, rpc::Message& result)
{
    if (!send_message(remote_port, msg))
        return false;

    for (;;)
    {
        PendingRpcMessage pending = next_message_();

        // We have to check the destination because next_message_ swaps source and destination ids
        // on message arrival.
        if (pending.message.destination == msg.destination && pending.message.opcode == msg.opcode &&
                remote_port == pending.source_port)
        {
            result = std::move(pending.message);
            break;
        }

        message_queue_.push_back(std::move(pending));
    }

    return true;
}

void Channel::next_id_()
{
    while (allocated_objects_.find(current_id_) != allocated_objects_.end())
        current_id_++;
}

}
