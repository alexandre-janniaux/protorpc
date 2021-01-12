#ifndef IPC_PORT_HH
#define IPC_PORT_HH

#include "protoipc/message.hh"

namespace ipc
{

    enum class PortError
    {
        Ok = 0,
        IncompleteMessage,
        TooManyHandles,
        ReadFailed,
        WriteFailed,
        BadFileDescriptor,
        PollError,
        Unknown
    };

    /**
     * Abstraction over platform specific ipc mechanism.
     */
    class Port
    {
    public:
        Port();
        Port(int fd);

        PortError send(const Message& message);
        PortError receive(Message& message);

        static bool create_pair(Port& a, Port& b);

        int handle() const
        {
            return pipe_fd_;
        }

        void close();

    private:
        int pipe_fd_;
    };
}

#endif
