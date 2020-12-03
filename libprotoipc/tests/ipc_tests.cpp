#include <thread>
#include <cstring>
#include <cstdlib>
#include <fcntl.h>
#include <sys/socket.h>
#include "gtest/gtest.h"

#include "protoipc/port.hh"
#include "protoipc/router.hh"

TEST(ipc_test, simple_send)
{
    ipc::Port source;
    ipc::Port destination;

    ASSERT_TRUE(ipc::Port::create_pair(source, destination));

    ipc::Message sent;
    sent.destination = 78;

    for (unsigned i = 0; i < 123; i++)
        sent.payload.push_back(0x41);

    ipc::PortError err = source.send(sent);

    ASSERT_EQ(err, ipc::PortError::Ok);

    ipc::Message received;

    err = destination.receive(received);

    ASSERT_EQ(err, ipc::PortError::Ok);
    ASSERT_EQ(received.destination, sent.destination);
    ASSERT_EQ(received.payload.size(), sent.payload.size());
    ASSERT_EQ(received.payload, sent.payload);
}

TEST(ipc_test, send_huge)
{
    ipc::Port source;
    ipc::Port destination;

    ASSERT_TRUE(ipc::Port::create_pair(source, destination));

    // 50Mb
    constexpr std::size_t PAYLOAD_SIZE = 50 * 1024 * 1024;

    ipc::Message sent;
    sent.payload.resize(PAYLOAD_SIZE);
    std::memset(sent.payload.data(), 0xfe, sent.payload.size());

    std::thread sending_thread([&source, &sent]() -> void {
        ipc::PortError err = source.send(sent);

        ASSERT_EQ(err, ipc::PortError::Ok);
    });

    ipc::Message received;
    ipc::PortError err = destination.receive(received);

    ASSERT_EQ(err, ipc::PortError::Ok);
    ASSERT_EQ(received.payload.size(), PAYLOAD_SIZE);
    ASSERT_EQ(received.payload, sent.payload);

    sending_thread.join();
}

TEST(ipc_test, router_simple)
{
    ipc::Port client_router_a;
    ipc::Port router_client_a;
    ipc::Port client_router_b;
    ipc::Port router_client_b;

    ASSERT_TRUE(ipc::Port::create_pair(client_router_a, router_client_a));
    ASSERT_TRUE(ipc::Port::create_pair(client_router_b, router_client_b));

    ipc::Router router;
    ipc::PortId client_a_id = router.add_port(router_client_a);
    ipc::PortId client_b_id = router.add_port(router_client_b);

    // Message from client a to client b
    std::vector<std::uint8_t> payload = { 0x41, 0x42, 0x43 };

    ipc::Message test;
    test.destination = client_b_id;
    test.payload = payload;

    std::thread send_message([&]() {
        client_router_a.send(test);
    });

    std::thread router_thread([&]() {
        router.loop();
    });

    // Leaking threads
    router_thread.detach();
    send_message.join();

    // We should receive the message with destination changed to client_a as it
    // passed through the router.
    ipc::Message received;
    ipc::PortError error = client_router_b.receive(received);

    ASSERT_EQ(error, ipc::PortError::Ok);
    ASSERT_EQ(received.destination, client_a_id);
    ASSERT_EQ(received.payload, payload);
}

#ifdef __linux__

TEST(ipc_test, fd_passing_linux)
{
    ipc::Port parent_port;
    ipc::Port child_port;

    ASSERT_TRUE(ipc::Port::create_pair(parent_port, child_port));

    int parent_pair[2];

    // Socket pair only present in the parent. We will pass through a message
    // one end of the pair to the child.
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, parent_pair), 0);

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
        ipc::Message received;
        ipc::PortError err = child_port.receive(received);

        ASSERT_EQ(err, ipc::PortError::Ok);
        ASSERT_EQ(received.handles.size(), 1);

        ipc::Port new_port(received.handles[0]);

        // Sending back data to ack the handle we received
        ipc::Message sent;
        sent.payload = { 0xde, 0xad, 0xbe, 0xef };

        err = new_port.send(sent);

        ASSERT_EQ(err, ipc::PortError::Ok);

        std::exit(0);
    }

    // Send handle that is absent from the child process.
    ipc::Message sent;
    sent.handles = { parent_b };

    ipc::PortError err = parent_port.send(sent);

    ASSERT_EQ(err, ipc::PortError::Ok);

    ipc::Port parent_a_port(parent_a);
    ipc::Message receive;
    err = parent_a_port.receive(receive);

    ASSERT_EQ(err, ipc::PortError::Ok);

    std::vector<std::uint8_t> expected_payload = { 0xde, 0xad, 0xbe, 0xef };
    ASSERT_EQ(receive.payload, expected_payload);
}

#endif

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
