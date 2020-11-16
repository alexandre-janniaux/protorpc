#ifndef IPC_MESSAGE_HH
#define IPC_MESSAGE_HH

#include <vector>
#include <cstdint>

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
}

#endif
