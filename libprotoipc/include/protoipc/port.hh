#ifndef IPC_PORT_HH
#define IPC_PORT_HH

#include <vector>
#include <cstdint>

#include "protoipc/message.hh"

namespace ipc
{
    /**
     * Abstraction over data sent over ports. Contains enough information
     * to be routed without having to parse the payload.
     */
    struct Message
    {
        std::uint64_t destination = 0;
        std::uint64_t opcode = 0;
        std::vector<std::uint8_t> payload;
        std::vector<int> handles;
    };

    enum class PortError
    {
        Ok = 0,
        IncompleteMessage,
        ReadFailed,
        WriteFailed,
        BadFileDescriptor,
        Unknown
    };

    /**
     * Abstraction over platform specific ipc mechanism.
     */
    class Port
    {
    public:
        Port(int fd);

        PortError send(const Message& message);
        PortError receive(Message& message);

        int handle() const
        {
            return pipe_fd_;
        }

    private:
        int pipe_fd_;
    };
}

#endif
