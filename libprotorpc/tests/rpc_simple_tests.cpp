#include <thread>
#include <sys/socket.h>
#include "gtest/gtest.h"
#include "fmt/core.h"

#include "protoipc/port.hh"
#include "protoipc/router.hh"
#include "protorpc/broker.hh"
#include "protorpc/channel.hh"
#include "protorpc/exchannel.hh"
#include "protorpc/serializer.hh"
#include "protorpc/unserializer.hh"

// Simple send test
constexpr std::uint64_t PING_COMMAND = 42;

class SimpleSendProxy : public rpc::RpcProxy
{
public:
    SimpleSendProxy(rpc::Channel* chan, std::uint64_t object, std::uint64_t remote)
        : rpc::RpcProxy(chan, object, remote)
    {}

    std::string ping(std::string token)
    {
        rpc::Serializer s;
        s.serialize(token);

        ipc::Message request;
        request.opcode = PING_COMMAND;
        request.destination = remote();
        request.payload = s.get_payload();

        ipc::Message reply = send_request(request);

        // Recover the pong
        rpc::Unserializer u(std::move(reply.payload));
        std::string result;

        if (!u.unserialize(&result))
            throw std::runtime_error("Could not unserialize pong");

        return result;
    }
};

class SimpleSendReceiver : public rpc::RpcReceiver
{
public:
    SimpleSendReceiver(rpc::Channel* chan, std::uint64_t object, std::uint64_t remote)
        : rpc::RpcReceiver(chan, object, remote)
    {}

    void on_message(ipc::Message& message) override
    {
        // (string ping) => (string pong)
        if (message.opcode == PING_COMMAND)
        {
            // We get the string sent by the proxy ...
            rpc::Unserializer u(std::move(message.payload));
            std::string message;

            if (!u.unserialize(&message))
                throw std::runtime_error("Could not unserialize ping string");

            // ... And we send it back around
            rpc::Serializer s;
            s.serialize(message);

            ipc::Message reply;
            reply.opcode = PING_COMMAND;
            reply.destination = remote();
            reply.payload = s.get_payload();

            send_message(reply);
        }
        else
            throw std::runtime_error("Invalid request");
    }
};

TEST(rpc_test, simple_send)
{
    int client_a_socks[2];
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_DGRAM, 0, client_a_socks), 0);

    int client_b_socks[2];
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_DGRAM, 0, client_b_socks), 0);

    ipc::Port broker_client_a_port(client_a_socks[0]);
    ipc::Port client_broker_a_port(client_a_socks[1]);
    ipc::Port broker_client_b_port(client_b_socks[0]);
    ipc::Port client_broker_b_port(client_b_socks[1]);

    // Setting up channels
    rpc::Channel first_channel(client_broker_a_port);
    rpc::Channel second_channel(client_broker_b_port);

    // Setting up the broker
    rpc::Broker broker;

    broker.add_port(broker_client_a_port);
    broker.add_port(broker_client_b_port);

    std::thread broker_thread([&]() {
        broker.loop();
    });

    constexpr std::uint64_t proxy_id = 22;
    constexpr std::uint64_t receiver_id = 23;

    // Bind a receiver to the second channel
    auto receiver = second_channel.bind<SimpleSendReceiver>(receiver_id, proxy_id);

    std::thread receiver_thread([&]() {
            second_channel.loop();
    });

    // Leaking threads ...
    broker_thread.detach();
    receiver_thread.detach();

    auto proxy = first_channel.bind<SimpleSendProxy>(proxy_id, receiver_id);

    std::string ping_msg = "eizruzboiczieiojzofczjooxjpokez";
    std::string pong = proxy->ping(ping_msg);

    ASSERT_EQ(pong, ping_msg);
}

// Ex api simple send

class ExSimpleSendProxy : public rpc::ExRpcProxy
{
public:
    ExSimpleSendProxy(rpc::ExChannel* chan, std::uint64_t object_id, std::uint64_t remote_port, std::uint64_t remote_id)
        : rpc::ExRpcProxy(chan, object_id, remote_port, remote_id)
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

class ExSimpleSendReceiver : public rpc::ExRpcReceiver
{
public:
    ExSimpleSendReceiver(rpc::ExChannel* chan, std::uint64_t object_id)
        : rpc::ExRpcReceiver(chan, object_id)
    {}

    void on_message(std::uint64_t source_port, rpc::Message& message) override
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
            channel_->send_message(source_port, message);
        }
    }
};

TEST(exrpc_test, simple_send)
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

    std::uint64_t client_a_id = router.add_port(router_client_a_port);
    std::uint64_t client_b_id = router.add_port(router_client_b_port);

    // Setting up channels
    rpc::ExChannel first_channel(client_a_id, client_router_a_port);
    rpc::ExChannel second_channel(client_b_id, client_router_b_port);

    auto receiver = second_channel.bind<ExSimpleSendReceiver>();
    auto proxy = first_channel.bind<ExSimpleSendProxy>(client_b_id, receiver->id());

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

TEST(exrpc_test, simple_multi_proxy)
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

    std::uint64_t client_a_id = router.add_port(router_client_a_port);
    std::uint64_t client_b_id = router.add_port(router_client_b_port);

    // Setting up channels
    rpc::ExChannel first_channel(client_a_id, client_router_a_port);
    rpc::ExChannel second_channel(client_b_id, client_router_b_port);

    auto receiver = second_channel.bind<ExSimpleSendReceiver>();

    auto first_proxy = first_channel.bind<ExSimpleSendProxy>(client_b_id, receiver->id());
    auto second_proxy = first_channel.bind<ExSimpleSendProxy>(client_b_id, receiver->id());

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
