#include <iostream>
#include <thread>
#include <sys/socket.h>
#include "protoipc/router.hh"
#include "calculator.sidl.hh"

// Implementing the receiver methods
namespace math
{

bool CalculatorReceiver::add(std::int64_t lhs, std::int64_t rhs, std::int64_t* result)
{
    *result = lhs + rhs;
    return true;
}

bool CalculatorReceiver::sub(std::int64_t lhs, std::int64_t rhs, std::int64_t* result)
{
    *result = lhs - rhs;
    return true;
}

bool CalculatorReceiver::mul(std::int64_t lhs, std::int64_t rhs, std::int64_t* result)
{
    *result = lhs * rhs;
    return true;
}

bool CalculatorReceiver::div(std::int64_t lhs, std::int64_t rhs, std::int64_t* result)
{
    *result = lhs / rhs;
    return true;
}

}

int main(int argc, char** argv)
{
    // Creating the sockets between elements
    int client_socks[2];
    socketpair(AF_UNIX, SOCK_DGRAM, 0, client_socks);

    int receiver_socks[2];
    socketpair(AF_UNIX, SOCK_DGRAM, 0, receiver_socks);

    ipc::Router router;
    std::uint64_t client_chan_id = router.add_port(ipc::Port(client_socks[0]));
    std::uint64_t receiver_chan_id = router.add_port(ipc::Port(receiver_socks[0]));

    // Setting up channels
    rpc::ExChannel client_chan(client_chan_id, ipc::Port(client_socks[1]));
    rpc::ExChannel receiver_chan(receiver_chan_id, ipc::Port(receiver_socks[1]));

    auto receiver = receiver_chan.bind<math::CalculatorReceiver>();
    auto proxy = client_chan.bind<math::CalculatorProxy>(receiver_chan_id, receiver->id());

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
