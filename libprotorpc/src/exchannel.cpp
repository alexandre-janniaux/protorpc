#include <stdexcept>
#include "protorpc/unserializer.hh"
#include "protorpc/exchannel.hh"

namespace rpc
{

using PendingRpcMessage = ExChannel::PendingRpcMessage;

ExChannel::ExChannel(std::uint64_t port_id, ipc::Port port)
    : port_id_(port_id), port_(port)
{}

std::uint64_t ExChannel::next_id_()
{
    while (receivers_.find(current_id_) != receivers_.end())
        current_id_++;

    return current_id_;
}

PendingRpcMessage ExChannel::next_message_()
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
        std::uint64_t payload_size = 0;

        status &= u.unserialize(&result.source);
        status &= u.unserialize(&result.destination);
        status &= u.unserialize(&result.opcode);
        status &= u.unserialize(&payload_size);

        if (!status)
            throw std::runtime_error("Could not decode rpc message header");

        result.payload = u.get_remaining();

        if (result.payload.size() != payload_size)
            throw std::runtime_error("Payload has invalid size");

        // We patch the rpc::Message to indicate the source object.
        pending.destination_object = result.destination;
        result.destination = result.source;
        result.source = pending.destination_object;

        pending.message = std::move(result);

        return pending;
}

void ExChannel::loop()
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

            handler->second->on_message(pending_msg.source_port, pending_msg.message);
            message_queue_.pop_front();
        }
    }
}

bool ExChannel::send_message(std::uint64_t remote_port, rpc::Message& msg)
{
    return true;
}

bool ExChannel::send_request(std::uint64_t remote_port, rpc::Message& msg, rpc::Message& result)
{
    return true;
}

}
