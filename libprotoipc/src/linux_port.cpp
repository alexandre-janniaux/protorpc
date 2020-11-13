#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "protoipc/port.hh"

// XXX: High enough limit for common cases (same as kMaxSendmsgHandles in mojo)
constexpr std::size_t MAX_HANDLES = 128;
// TODO: Use getsockopt in Port constructor to query the best value
constexpr std::size_t MSG_MAX_SIZE = 8192;

namespace ipc
{

Port::Port(int fd)
    : pipe_fd_(fd)
{}

PortError Port::send(std::vector<std::uint8_t>& data, std::vector<int>& handles)
{
    // First we need to send payload size + handle count + handles
    std::uint8_t sendmsg_payload[CMSG_SPACE(sizeof(int) * MAX_HANDLES * 2)];
    std::size_t header_payload[2] = { data.size(), handles.size() };

    struct msghdr header = {0};
    struct iovec iov;

    iov.iov_base = &header_payload[0];
    iov.iov_len = sizeof(header_payload);

    header.msg_iov = &iov;
    header.msg_iovlen = 1;

    if (handles.size() > 0)
    {
        header.msg_control = sendmsg_payload;
        header.msg_controllen = CMSG_SPACE(sizeof(int) * handles.size());

        // Prepare fds
        struct cmsghdr* cmsg = CMSG_FIRSTHDR(&header);
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_type = SCM_RIGHTS;
        cmsg->cmsg_len = CMSG_LEN(sizeof(int) * handles.size());

        std::memcpy(CMSG_DATA(cmsg), handles.data(), sizeof(int) * handles.size());
    }

    int err = sendmsg(pipe_fd_, &header, 0);

    if (err < 0)
    {
        if (errno == EBADF)
            return PortError::BadFileDescriptor;

        return PortError::Unknown;
    }

    std::size_t written = 0;

    while (written != data.size())
    {
        std::size_t remaining = data.size() - written;
        remaining = (remaining > MSG_MAX_SIZE) ? MSG_MAX_SIZE : remaining;

        ssize_t cur_written = ::send(pipe_fd_, data.data() + written, remaining, 0);

        if (cur_written < 0)
        {
            if (errno == EINTR)
                continue;

            if (errno == EBADF)
                return PortError::BadFileDescriptor;

            return PortError::WriteFailed;
        }

        written += cur_written;
    }

    return PortError::Ok;
}

PortError Port::receive(std::vector<std::uint8_t>& data, std::vector<int>& handles)
{
    data.clear();
    handles.clear();

    // First we receive payload size + handle count + handles
    // We allocate enough space just to be safe
    std::uint8_t recvmsg_payload[CMSG_SPACE(sizeof(int) * MAX_HANDLES * 2)];
    std::size_t header_payload[2] = {0};

    struct msghdr header = {0};
    struct iovec iov;

    iov.iov_base = &header_payload[0];
    iov.iov_len = sizeof(header_payload);

    header.msg_iov = &iov;
    header.msg_iovlen = 1;
    header.msg_control = recvmsg_payload;
    header.msg_controllen = sizeof(recvmsg_payload);

    int err = recvmsg(pipe_fd_, &header, 0);

    if (err < 0)
    {
        if (errno == EBADF)
            return PortError::BadFileDescriptor;

        return PortError::Unknown;
    }

    // Collect handles
    std::size_t payload_size = header_payload[0];
    std::size_t handle_count = header_payload[1];

    if (handle_count > 0)
    {
        struct cmsghdr* cmsg = CMSG_FIRSTHDR(&header);

        if (!cmsg)
            return PortError::IncompleteMessage;

        handles.resize(handle_count);
        std::memcpy(handles.data(), CMSG_DATA(cmsg), handle_count * sizeof(int));
    }

    // Now read the actual payload into the output buffer. We do this in two
    // steps to not be limited by the 200k transfer size limit on sendmsg.
    data.resize(payload_size);
    std::size_t bytes_read = 0;

    while (bytes_read != payload_size)
    {
        std::size_t remaining = payload_size - bytes_read;
        remaining = (remaining > MSG_MAX_SIZE) ? MSG_MAX_SIZE : remaining;

        ssize_t cur_read = recv(pipe_fd_, data.data() + bytes_read, remaining, 0);

        if (cur_read < 0)
        {
            if (errno == EINTR)
                continue;

            return PortError::ReadFailed;
        }

        bytes_read += cur_read;
    }

    return PortError::Ok;
}

}
