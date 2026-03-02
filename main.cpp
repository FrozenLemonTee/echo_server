#include "acceptor.h"
#include "socket.h"
#include "endpoint.h"
#include "coroutines.h"
#include "executors.h"
#include "singleton.h"
#include "zeit.h"
#include <iostream>
#include <array>
#include <string>

constexpr uint16_t PORT = 12345;

std::string prefix(const std::string_view msg)
{
    std::stringstream ss;
    const auto now = original::time::UTCTime::localNow();
    ss << "[" << msg.data() << " " << now.timeComponents() << " " << now.calendarComponents() << "]";
    return ss.str();
}

void run_server()
{
    const std::string addr = "127.0.0.1";
    original::endpoint ep(original::sockets::IPV4, addr, PORT);

    original::singleton<original::taskDelegator>::reset();
    auto& delegator = original::singleton<original::taskDelegator>::instance();

    original::singleton<original::threadPoolExecutor>::reset(delegator);
    auto& executor = original::singleton<original::threadPoolExecutor>::instance();

    auto server_task = [ep, addr]() -> void {

        std::cout << prefix("server") << " listening on: " << addr << ":" << PORT << std::endl;

        const original::acceptor a(ep);
        const original::socket client = a.accept();

        std::cout << prefix("server") << " client connected\n";

        std::array<original::byte, 1024> buf{};

        while (true)
        {
            const auto n = client.recv({buf.data(), sizeof(buf)});
            if (n == 0)
            {
                std::cout << prefix("server") << " client disconnected\n";
                break;
            }

            std::string msg(reinterpret_cast<const char*>(buf.data()), n);
            std::cout << prefix("server") << " received: " << msg << "\n";

            static_cast<void>(client.send({buf.data(), static_cast<original::u_integer>(n)}));
        }
    };

    const auto task = executor >> server_task;

    if (const auto res = task.start(); !res)
        throw std::runtime_error("task start failed");

    while (!task.finished())
        original::thread::yield();
}

void run_client()
{
    const std::string addr = "127.0.0.1";
    const original::endpoint ep(original::sockets::IPV4, addr, PORT);

    const original::socket client(original::sockets::IPV4,
                  original::sockets::STREAM,
                  original::sockets::TCP);

    client.connect(ep);

    std::cout << prefix("client") << " connected to server " << addr << std::endl;

    while (true)
    {
        std::cout << "> ";
        std::string input;

        if (!std::getline(std::cin, input))
            break;

        if (input == "exit")
            break;

        static_cast<void>(client.send({reinterpret_cast<const byte*>(input.data()), static_cast<original::u_integer>(input.size())}));

        std::array<char, 1024> buf{};
        const auto n = client.recv({reinterpret_cast<byte*>(buf.data()), sizeof(buf)});

        std::string resp(buf.data(), n);
        std::cout << prefix("client echo") << resp << "\n";
    }

    std::cout << prefix("client") << " exiting\n";
}

int main(const int argc, char** argv)
{
    if (argc < 2)
    {
        std::cout << "usage:\n";
        std::cout << "  ./echo_app server\n";
        std::cout << "  ./echo_app client\n";
        return 0;
    }

    if (const std::string mode = argv[1]; mode == "server")
    {
        run_server();
    }
    else if (mode == "client")
    {
        run_client();
    }
    else
    {
        std::cout << "unknown mode\n";
    }

    return 0;
}