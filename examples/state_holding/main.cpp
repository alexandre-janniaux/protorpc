#include <iostream>
#include <thread>
#include <sys/socket.h>
#include <optional>
#include <map>
#include "protoipc/router.hh"
#include "database.sidl.hh"

namespace db
{
    class DatabaseReceiverImpl: public DatabaseReceiver
    {
    public:
        DatabaseReceiverImpl(rpc::Channel* chan, std::uint64_t object_id, std::string version)
            : DatabaseReceiver(chan, object_id)
        {
            data_["version"] = version;
        }

        bool add(std::string key, std::string value) override
        {
            data_[key] = value;
            return true;
        }

        bool get(std::string key, std::optional<std::string>* result) override
        {
            auto it = data_.find(key);

            if (it == data_.end())
                *result = std::nullopt;
            else
                *result = it->second;

            return true;
        }

    private:
        std::map<std::string, std::string> data_;
    };
}

namespace db
{

bool DatabaseReceiver::add(std::string, std::string)
{
    return true;
}

bool DatabaseReceiver::get(std::string, std::optional<std::string>*)
{
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
    rpc::Channel client_chan(client_chan_id, ipc::Port(client_socks[1]));
    rpc::Channel receiver_chan(receiver_chan_id, ipc::Port(receiver_socks[1]));

    auto receiver = receiver_chan.bind<db::DatabaseReceiverImpl>("0.0.1-beta");
    auto proxy = client_chan.bind<db::DatabaseProxy>(receiver_chan_id, receiver->id());

    std::thread router_thread([&]() {
        router.loop();
    });

    router_thread.detach();

    std::thread receiver_thread([&]() {
        receiver_chan.loop();
    });

    receiver_thread.detach();

    // Using the proxy
    std::optional<std::string> value;

    if (!proxy->get("version", &value))
    {
        std::cout << "proxy->get(...) failed\n";
        return 1;
    }

    if (!value)
    {
        std::cout << "proxy->get(\"version\") should not be empty\n";
        return 1;
    }

    std::cout << "Version: " << *value << '\n';

    return 0;
}
