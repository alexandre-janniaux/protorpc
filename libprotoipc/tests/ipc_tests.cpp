#include <thread>
#include <cstring>
#include <cstdlib>
#include <fcntl.h>
#include <sys/socket.h>
#include "gtest/gtest.h"

#include "protoipc/port.hh"

TEST(ipc_test, simple_send)
{
    int pair[2];
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_DGRAM, 0, pair), 0);

    ipc::Port source(pair[0]);
    ipc::Port destination(pair[1]);

    std::vector<std::uint8_t> input_data;
    std::vector<int> dummy_input_handles;

    for (unsigned i = 0; i < 123; i++)
        input_data.push_back(0x41);

    ipc::PortError err = source.send(input_data, dummy_input_handles);

    ASSERT_EQ(err, ipc::PortError::Ok);

    std::vector<std::uint8_t> output_data;
    std::vector<int> dummy_output_handles;

    err = destination.receive(output_data, dummy_output_handles);

    ASSERT_EQ(err, ipc::PortError::Ok);
    ASSERT_EQ(output_data.size(), input_data.size());
    ASSERT_EQ(output_data, input_data);
}

TEST(ipc_test, send_huge)
{
    int pair[2];
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_DGRAM, 0, pair), 0);

    ipc::Port source(pair[0]);
    ipc::Port destination(pair[1]);

    // 50Mb
    constexpr std::size_t PAYLOAD_SIZE = 50 * 1024 * 1024;

    std::vector<std::uint8_t> input_payload;
    input_payload.resize(PAYLOAD_SIZE);
    std::memset(input_payload.data(), 0xfe, input_payload.size());

    std::thread sending_thread([&source, &input_payload]() -> void {
        std::vector<int> dummy_handles;
        ipc::PortError err = source.send(input_payload, dummy_handles);

        ASSERT_EQ(err, ipc::PortError::Ok);
    });

    std::vector<std::uint8_t> output;
    std::vector<int> dummy_output_handles;

    ipc::PortError err = destination.receive(output, dummy_output_handles);

    ASSERT_EQ(err, ipc::PortError::Ok);
    ASSERT_EQ(output.size(), PAYLOAD_SIZE);
    ASSERT_EQ(output, input_payload);

    sending_thread.join();
}

#ifdef __linux__

TEST(ipc_test, fd_passing_linux)
{
    int pair[2];
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_DGRAM, 0, pair), 0);

    ipc::Port parent_port(pair[0]);
    ipc::Port child_port(pair[1]);

    int parent_pair[2];

    // Socket pair only present in the parent. We will pass through a message
    // one end of the pair to the child.
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_DGRAM, 0, parent_pair), 0);

    int parent_a = fcntl(parent_pair[0], F_DUPFD_CLOEXEC, 0);
    int parent_b = fcntl(parent_pair[1], F_DUPFD_CLOEXEC, 0);

    ASSERT_NE(parent_a, -1);
    ASSERT_NE(parent_b, -1);

    close(parent_pair[0]);
    close(parent_pair[1]);

    int pid = fork();

    // Child process
    if (pid == 0)
    {
        std::vector<std::uint8_t> payload;
        std::vector<int> handles;

        ipc::PortError err = child_port.receive(payload, handles);

        ASSERT_EQ(err, ipc::PortError::Ok);
        ASSERT_EQ(handles.size(), 1);

        ipc::Port new_port(handles[0]);
        payload = { 0xde, 0xad, 0xbe, 0xef };
        handles.clear();

        err = new_port.send(payload, handles);

        ASSERT_EQ(err, ipc::PortError::Ok);

        std::exit(0);
    }

    // Send handle that is absent from the child process.
    std::vector<std::uint8_t> payload;
    std::vector<int> handles = { parent_b };

    ipc::PortError err = parent_port.send(payload, handles);

    ASSERT_EQ(err, ipc::PortError::Ok);

    ipc::Port parent_a_port(parent_a);
    err = parent_a_port.receive(payload, handles);

    ASSERT_EQ(err, ipc::PortError::Ok);

    std::vector<std::uint8_t> expected_payload = { 0xde, 0xad, 0xbe, 0xef };
    ASSERT_EQ(payload, expected_payload);
}

#endif


int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
