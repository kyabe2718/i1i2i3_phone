//
// sender.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <sstream>
#include <string>
#include <boost/asio.hpp>
#include "boost/bind.hpp"
#include "boost/date_time/posix_time/posix_time_types.hpp"

const short multicast_port = 12345;
const int max_message_count = 1000;

class sender
{
public:
    sender(boost::asio::io_service& io_service,
        const boost::asio::ip::address& multicast_address,
        const boost::asio::ip::address_v4& network_interface)
        : endpoint_(multicast_address, multicast_port),
          socket_(io_service, endpoint_.protocol()),
          timer_(io_service),
          message_count_(0)
    {
        socket_.set_option(boost::asio::ip::multicast::outbound_interface(network_interface));
        socket_.set_option(boost::asio::ip::multicast::enable_loopback(true));

        std::ostringstream os;
        os << "Message " << message_count_++;
        message_ = os.str();

        socket_.async_send_to(
            boost::asio::buffer(message_), endpoint_,
            boost::bind(&sender::handle_send_to, this,
                boost::asio::placeholders::error));

        std::cout << message_ << std::endl;
    }

    void handle_send_to(const boost::system::error_code& error)
    {
        if (!error && message_count_ < max_message_count) {
            timer_.expires_from_now(boost::posix_time::seconds(1));
            timer_.async_wait(
                boost::bind(&sender::handle_timeout, this,
                    boost::asio::placeholders::error));
        }
    }

    void handle_timeout(const boost::system::error_code& error)
    {
        if (!error) {
            std::ostringstream os;
            os << "Message " << message_count_++;
            message_ = os.str();

            socket_.async_send_to(
                boost::asio::buffer(message_), endpoint_,
                boost::bind(&sender::handle_send_to, this,
                    boost::asio::placeholders::error));
            std::cout << message_ << std::endl;
        }
    }

private:
    boost::asio::ip::udp::endpoint endpoint_;
    boost::asio::ip::udp::socket socket_;
    boost::asio::deadline_timer timer_;
    int message_count_;
    std::string message_;
};

int main(int argc, char* argv[])
{
    try {
        if (argc != 3) {
            std::cerr << "Usage: sender <multicast_address> <network_interface>\n";
            std::cerr << "  For IPv4, try:\n";
            std::cerr << "    sender 239.255.0.1\n";
            std::cerr << "  For IPv6, try:\n";
            std::cerr << "    sender ff31::8000:1234\n";
            return 1;
        }

        boost::asio::io_service io_service;
        sender s(io_service, boost::asio::ip::address::from_string(argv[1]), boost::asio::ip::address_v4::from_string(argv[2]));
        io_service.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
