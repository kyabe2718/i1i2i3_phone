#include "gateway.hpp"


int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cerr << "usage: server [port]" << std::endl;
        return 1;
    }

    unsigned short port = std::atoi(argv[1]);


    namespace asio = boost::asio;
    asio::io_context io_context;

    auto gateway = std::make_shared<IpPhone::GateWay>(io_context, port);

    gateway->start_accept();

    io_context.run();

    return 0;
}
