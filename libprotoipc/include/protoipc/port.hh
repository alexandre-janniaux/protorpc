#ifndef IPC_PORT_HH
#define IPC_PORT_HH

#include <vector>
#include <cstdint>

namespace ipc
{
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

        PortError send(std::vector<std::uint8_t>& data, std::vector<int>& handles);
        PortError receive(std::vector<std::uint8_t>& data, std::vector<int>& handles);

        int handle() const
        {
            return pipe_fd_;
        }

    private:
        int pipe_fd_;
    };
}

#endif
