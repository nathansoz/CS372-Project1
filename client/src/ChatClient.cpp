//
// Created by nathan on 7/22/17.
//

#include <iostream>
#include "../include/ChatClient.h"

ChatClient::ChatClient()
{
}

ChatClient::~ChatClient()
{

}

void ChatClient::Connect(int port)
{
    std::cout << "Connecting on port " << port << std::endl;
    auto addr = boost::asio::ip::address::from_string("127.0.0.1");
    boost::asio::ip::tcp::endpoint ep(addr, 8080);
    boost::asio::ip::tcp::socket socket(io_service);

    socket.connect(ep);

    std::string test = u8"TEST \xF0\x9F\x98\xBB";

    const char* buffer = test.c_str();

    uint32_t size = (uint32_t)test.length();

    uint32_t sendsize = htonl(size);

    boost::asio::write(socket, boost::asio::buffer(&sendsize, sizeof(uint32_t)));
    boost::asio::write(socket, boost::asio::buffer(buffer, test.length()));
    socket.close();
}

