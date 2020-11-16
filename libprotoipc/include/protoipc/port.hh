#ifndef IPC_PORT_HH
#define IPC_PORT_HH

#include "protoipc/message.hh"

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
