#ifndef RPC_MESSAGE_HH
#define RPC_MESSAGE_HH

#include <vector>
#include <cstdint>

namespace rpc
{

    struct Message
    {
        // Source object
        std::uint64_t source;

        // Destination object
        std::uint64_t destination;

        // Operation on the object
        std::uint64_t opcode;

        // rpc message content
        std::vector<std::uint8_t> payload;

        // File descriptors
        std::vector<int> handles;
    };
}

#endif
