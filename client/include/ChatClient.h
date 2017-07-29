//
// Created by nathan on 7/22/17.
//

#ifndef CLIENT_CHATCLIENT_H
#define CLIENT_CHATCLIENT_H

#include <boost/asio.hpp>

class ChatClient
{
public:
    ChatClient();
    ~ChatClient();
    void Connect(int port);
private:
    boost::asio::io_service io_service;

};

#endif //CLIENT_CHATCLIENT_H
