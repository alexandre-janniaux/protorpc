#include <thread>
#include <sys/socket.h>
#include "gtest/gtest.h"
#include "fmt/core.h"

#include "protoipc/port.hh"
#include "protoipc/router.hh"
#include "protorpc/channel.hh"
#include "protorpc/serializer.hh"
#include "protorpc/unserializer.hh"

constexpr std::uint64_t PING_COMMAND = 42;

class SimpleSendProxy : public rpc::RpcProxy
{
public:
    SimpleSendProxy(rpc::Channel* chan, rpc::ObjectId object_id, rpc::PortId remote_port, rpc::ObjectId remote_id)
        : rpc::RpcProxy(chan, object_id, remote_port, remote_id)
    {}

    bool ping(std::string ping_str, std::string* output)
    {
        rpc::Message message;
        message.source = id();
        message.destination = remote_id();
        message.opcode = PING_COMMAND;

        rpc::Serializer s;
        s.serialize(ping_str);

        message.payload = s.get_payload();

        rpc::Message result;

        if (!channel_->send_request(remote_port(), message, result))
        {
            fmt::print("[PROXY] There was an error when sending the ping request\n");
            return false;
        }

        rpc::Unserializer u(std::move(result.payload));

        if (!u.unserialize(output))
            throw std::runtime_error("There was an error parsing the ping reply");

        return true;
    }
};

class SimpleSendReceiver : public rpc::RpcReceiver
{
public:
    void on_message(rpc::Channel& chan, rpc::ObjectId object, rpc::PortId source_port, rpc::Message& message) override
    {
        if (message.opcode == PING_COMMAND)
        {
            fmt::print("Received PING from {},{}\n", source_port, message.destination);

            rpc::Unserializer u(std::move(message.payload));
            std::string ping_str;

            if (!u.unserialize(&ping_str))
                throw std::runtime_error("Could not unserialize ping string");

            rpc::Serializer s;
            s.serialize(ping_str);

            message.payload = s.get_payload();
            chan.send_message(source_port, message);
        }
    }
};

TEST(rpc_test, simple_send)
{
    int client_a_socks[2];
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_DGRAM, 0, client_a_socks), 0);

    int client_b_socks[2];
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_DGRAM, 0, client_b_socks), 0);

    ipc::Port router_client_a_port(client_a_socks[0]);
    ipc::Port client_router_a_port(client_a_socks[1]);
    ipc::Port router_client_b_port(client_b_socks[0]);
    ipc::Port client_router_b_port(client_b_socks[1]);

    // Setting up the router between clients
    ipc::Router router;

    rpc::PortId client_a_id = router.add_port(router_client_a_port);
    rpc::PortId client_b_id = router.add_port(router_client_b_port);

    // Setting up channels
    rpc::Channel first_channel(client_a_id, client_router_a_port);
    rpc::Channel second_channel(client_b_id, client_router_b_port);

    auto receiver_id = second_channel.bind<SimpleSendReceiver>();
    auto proxy = first_channel.connect<SimpleSendProxy>(client_b_id, receiver_id);

    std::thread router_thread([&]() {
        router.loop();
    });

    std::thread receiver_thread([&]() {
        second_channel.loop();
    });

    // Leaking threads
    router_thread.detach();
    receiver_thread.detach();

    std::string ping_string = "7253c09bd391db2cd370455fc64e520ac79fca31";
    std::string pong_string;

    bool result = proxy->ping(ping_string, &pong_string);

    ASSERT_EQ(result, true);
    ASSERT_EQ(pong_string, ping_string);
}

TEST(rpc_test, simple_multi_proxy)
{
    int client_a_socks[2];
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_DGRAM, 0, client_a_socks), 0);

    int client_b_socks[2];
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_DGRAM, 0, client_b_socks), 0);

    ipc::Port router_client_a_port(client_a_socks[0]);
    ipc::Port client_router_a_port(client_a_socks[1]);
    ipc::Port router_client_b_port(client_b_socks[0]);
    ipc::Port client_router_b_port(client_b_socks[1]);

    // Setting up the router between clients
    ipc::Router router;

    rpc::PortId client_a_id = router.add_port(router_client_a_port);
    rpc::PortId client_b_id = router.add_port(router_client_b_port);

    // Setting up channels
    rpc::Channel first_channel(client_a_id, client_router_a_port);
    rpc::Channel second_channel(client_b_id, client_router_b_port);

    auto receiver_id = second_channel.bind<SimpleSendReceiver>();

    auto first_proxy = first_channel.connect<SimpleSendProxy>(client_b_id, receiver_id);
    auto second_proxy = first_channel.connect<SimpleSendProxy>(client_b_id, receiver_id);

    ASSERT_NE(first_proxy->id(), second_proxy->id());

    std::thread router_thread([&]() {
        router.loop();
    });

    std::thread receiver_thread([&]() {
        second_channel.loop();
    });

    // Leaking threads
    router_thread.detach();
    receiver_thread.detach();

    // First proxy
    std::string ping_string = "7253c09bd391db2cd370455fc64e520ac79fca31";
    std::string pong_string;

    bool result = first_proxy->ping(ping_string, &pong_string);

    ASSERT_EQ(result, true);
    ASSERT_EQ(pong_string, ping_string);

    // Second proxy
    ping_string = "8642f7ca4cbc8318b82d44b07e593a49fd35c4bc";
    pong_string = "";

    result = second_proxy->ping(ping_string, &pong_string);

    ASSERT_EQ(result, true);
    ASSERT_EQ(pong_string, ping_string);
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
