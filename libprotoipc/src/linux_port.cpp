#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "protoipc/port.hh"

// XXX: High enough limit for common cases (same as kMaxSendmsgHandles in mojo)
constexpr std::size_t IPC_MAX_HANDLES = 128;

namespace ipc
{

Port::Port()
    : pipe_fd_(-1)
{}

Port::Port(int fd)
    : pipe_fd_(fd)
{}

/**
 * Sends a message over a native port.
 *
 * The message is sent over two iovecs. The first iovec contains the ipc header
 * composed of [payload_size, handle_count, destination]. The second iovec
 * contains the actual payload.
 */
PortError Port::send(const Message& message)
{
    char sendmsg_control[CMSG_SPACE(sizeof(int) * IPC_MAX_HANDLES * 2)] = {0};
    std::uint64_t ipc_header[] = {
        message.payload.size(),
        message.handles.size(),
        message.destination
    };

    struct msghdr header = {};
    struct iovec iov[2];

    // Header iovec
    iov[0].iov_base = ipc_header;
    iov[0].iov_len  = sizeof(ipc_header);

    // Data iovec
    iov[1].iov_base = const_cast<std::uint8_t*>(message.payload.data());
    iov[1].iov_len  = message.payload.size();

    header.msg_iov = iov;
    header.msg_iovlen = 2;

    if (message.handles.size() > 0)
    {
        header.msg_control = sendmsg_control;
        header.msg_controllen = CMSG_SPACE(sizeof(int) * message.handles.size());

        struct cmsghdr* cmsg = CMSG_FIRSTHDR(&header);
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_type = SCM_RIGHTS;
        cmsg->cmsg_len = CMSG_LEN(sizeof(int) * message.handles.size());

        std::memcpy(CMSG_DATA(cmsg), message.handles.data(), sizeof(int) * message.handles.size());
    }

    int err = 0;

    while((err = sendmsg(pipe_fd_, &header, 0)) == -1)
    {
        if (errno == EINTR)
            continue;
        else if (errno == EBADF)
            return PortError::BadFileDescriptor;
        else
            return PortError::Unknown;
    }

    return PortError::Ok;
}

/**
 * Receives a message from a native port.
 */
PortError Port::receive(Message& message)
{
    char recvmsg_control[CMSG_SPACE(sizeof(int) * IPC_MAX_HANDLES * 2)];
    std::uint64_t ipc_header[3];

    struct msghdr header = {};
    struct iovec iov[2];

    // Header iovec
    iov[0].iov_base = ipc_header;
    iov[0].iov_len  = sizeof(ipc_header);

    // Data iovec (temporary)
    iov[1].iov_base = NULL;
    iov[1].iov_len = 0;

    header.msg_iov = iov;
    header.msg_iovlen = 2;

    int err = 0;

    while ((err = recvmsg(pipe_fd_, &header, MSG_PEEK)) == -1)
    {
        if (errno == EINTR)
            continue;
        else if (errno == EBADF)
            return PortError::BadFileDescriptor;
        else
            return PortError::Unknown;
    }

    header.msg_control = recvmsg_control;
    header.msg_controllen = sizeof(recvmsg_control);

    message.payload.resize(ipc_header[0]);
    message.handles.resize(ipc_header[1]);
    message.destination = ipc_header[2];

    iov[1].iov_base = message.payload.data();
    iov[1].iov_len  = message.payload.size();

    while ((err = recvmsg(pipe_fd_, &header, MSG_WAITALL)) == -1)
    {
        if (errno == EINTR)
            continue;
        else if (errno == EBADF)
            return PortError::BadFileDescriptor;
        else
            return PortError::Unknown;
    }

    if (message.handles.size() > 0)
    {
        struct cmsghdr* cmsg = CMSG_FIRSTHDR(&header);

        if (!cmsg)
            return PortError::IncompleteMessage;

        std::memcpy(message.handles.data(), CMSG_DATA(cmsg), sizeof(int) * message.handles.size());
    }

    return PortError::Ok;
}

bool Port::create_pair(Port& a, Port& b)
{
    int pair[2];

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, pair) == -1)
        return false;

    a = Port(pair[0]);
    b = Port(pair[1]);

    return true;
}

void Port::close()
{
    ::close(pipe_fd_);
    pipe_fd_ = -1;
}

}
