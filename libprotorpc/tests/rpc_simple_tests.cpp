#include <thread>
#include <sys/socket.h>
#include "gtest/gtest.h"
#include "fmt/core.h"

#include "protoipc/port.hh"
#include "protorpc/broker.hh"
#include "protorpc/channel.hh"
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
        request.payload = s.get();

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
            reply.payload = s.get();

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
    auto receiver = second_channel.bind_receiver<SimpleSendReceiver>(receiver_id, proxy_id);

    std::thread receiver_thread([&]() {
            second_channel.loop();
    });

    // Leaking threads ...
    broker_thread.detach();
    receiver_thread.detach();

    auto proxy = first_channel.bind_proxy<SimpleSendProxy>(proxy_id, receiver_id);

    std::string ping_msg = "eizruzboiczieiojzofczjooxjpokez";
    std::string pong = proxy->ping(ping_msg);

    ASSERT_EQ(pong, ping_msg);
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
