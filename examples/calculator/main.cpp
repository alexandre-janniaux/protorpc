#include <iostream>
#include <thread>
#include <sys/socket.h>
#include "protoipc/router.hh"
#include "calculator.sidl.hh"

// Implementing the receiver methods
namespace math
{
    class CalculatorImpl: public CalculatorReceiver
    {
    public:
        bool add(std::int64_t lhs, std::int64_t rhs, std::int64_t* result) override
        {
            *result = lhs + rhs;
            return true;
        }

        bool sub(std::int64_t lhs, std::int64_t rhs, std::int64_t* result) override
        {
            *result = lhs - rhs;
            return true;
        }

        bool mul(std::int64_t lhs, std::int64_t rhs, std::int64_t* result) override
        {
            *result = lhs * rhs;
            return true;
        }

        bool div(std::int64_t lhs, std::int64_t rhs, std::int64_t* result) override
        {
            *result = lhs / rhs;
            return true;
        }
    };
}

int main(int argc, char** argv)
{
    // Creating the sockets between elements
    int client_socks[2];
    socketpair(AF_UNIX, SOCK_DGRAM, 0, client_socks);

    int receiver_socks[2];
    socketpair(AF_UNIX, SOCK_DGRAM, 0, receiver_socks);

    ipc::Router router;
    rpc::PortId client_chan_id = router.add_port(ipc::Port(client_socks[0]));
    rpc::PortId receiver_chan_id = router.add_port(ipc::Port(receiver_socks[0]));

    // Setting up channels
    rpc::Channel client_chan(client_chan_id, ipc::Port(client_socks[1]));
    rpc::Channel receiver_chan(receiver_chan_id, ipc::Port(receiver_socks[1]));

    rpc::ObjectId receiver_id = receiver_chan.bind<math::CalculatorImpl>();
    auto proxy = client_chan.connect<math::CalculatorProxy>(receiver_chan_id, receiver_id);

    std::thread router_thread([&]() {
        router.loop();
    });

    router_thread.detach();

    std::thread receiver_thread([&]() {
        receiver_chan.loop();
    });

    receiver_thread.detach();

    // Now doing computations
    std::int64_t lhs = 2;
    std::int64_t rhs = 40;
    std::int64_t result = 0;

    if (!proxy->add(lhs, rhs, &result))
    {
        std::cout << "proxy->add(...) failed\n";
        return 1;
    }

    std::cout << "proxy->add(...) = " << result << '\n';

    return 0;
}
