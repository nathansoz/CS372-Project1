//
// Created by nathan on 7/22/17.
//

#include <iostream>
#include <cstring>
#include <chrono>
#include <thread>

#include "../include/ChatClient.h"
#include "../include/Constants.h"

ChatClient::ChatClient(std::string handle)
{
    _disconnecting = false;

    _socketFileDescriptor = socket(AF_UNIX, SOCK_STREAM, 0);

    if(_socketFileDescriptor < 0)
    {
        std::cout << "Unable to create socket! Exiting." << std::endl;
        exit(ERR_CREATE_SOCKET);
    }

    _sendQueue.emplace(handle);
}

ChatClient::~ChatClient()
{

}

void ChatClient::Disconnect()
{
    _disconnecting = true;

    while(!_sendWorker.joinable())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    _sendWorker.join();
}

void ChatClient::Connect(int port)
{
    std::cout << "Connecting on port " << port << std::endl;

    struct addrinfo hints, *res, *loop;
    int status;
    char ipstr[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo("127.0.0.1", "8080", &hints, &res)) != 0 || res == nullptr)
    {
        std::cerr << "getaddrinfo failed with: " << gai_strerror(status) << std::endl;
        exit(1);
    }

    //This for loop for error checking based on:
    //    http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html#clientserver
    for(loop = res; loop != nullptr; loop = res->ai_next)
    {
        _socketFileDescriptor = socket(loop->ai_family, loop->ai_socktype, loop->ai_protocol);

        if(_socketFileDescriptor == -1)
        {
            continue;
        }

        if(connect(_socketFileDescriptor, loop->ai_addr, loop->ai_addrlen) == -1)
        {
            close(_socketFileDescriptor);
            continue;
        }

        break;
    }

    //did we get to the end of the loop without connecting?
    if(loop == nullptr)
    {
        std::cerr << "Failed to connect to server." << std::endl;
        exit(ERR_CONNECT);
    }

    _sendWorker = std::thread(&ChatClient::SendLoop, this);
    _receiveWorker = std::thread(&ChatClient::ReceiveLoop, this);
}

void ChatClient::ReceiveLoop()
{
    while(true)
    {
        size_t messageSizeLength = sizeof(uint32_t);
        size_t messageSizeRemaining = messageSizeLength;

        uint32_t messageSize;

        do
        {
            messageSizeRemaining -= recv(_socketFileDescriptor, &messageSize + (messageSizeLength - messageSizeRemaining), messageSizeRemaining, 0);
        }
        while(messageSizeRemaining > 0);

        messageSize = ntohl(messageSize);

        if(messageSize <= 0)
            continue;

        size_t messageRemaining = messageSize;
        std::unique_ptr<char[]> buffer = std::make_unique<char[]>(messageSize + 1);

        do
        {
            messageRemaining -= recv(_socketFileDescriptor, buffer.get() + (messageSize - messageRemaining), messageRemaining, 0);
        } while(messageRemaining > 0);

        buffer[messageSize] = '\n';
        std::string toPrint(buffer.get());

        std::cout << toPrint << std::endl;
    }
}

void ChatClient::SendLoop()
{
    while(true)
    {
        if(_sendQueueLock.try_lock())
        {
            if(_sendQueue.empty())
            {
                _sendQueueLock.unlock();
                if(!_disconnecting)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    continue;
                }

                break;
            }
            std::string toSend(_sendQueue.front());
            _sendQueue.pop();
            _sendQueueLock.unlock();

            const char* buffer = toSend.c_str();

            uint32_t bodySize = (uint32_t)toSend.length();
            uint32_t sendSize = htonl(bodySize);

            size_t headerSize = sizeof(uint32_t);
            size_t headerRemaining = sizeof(uint32_t);
            do
            {
                headerRemaining -= send(_socketFileDescriptor, &sendSize + (headerSize - headerRemaining), headerRemaining, 0);
            }
            while(headerRemaining > 0);


            uint32_t bodyRemaining = bodySize;
            do
            {
                bodyRemaining -= send(_socketFileDescriptor, buffer + (bodySize - bodyRemaining), bodyRemaining, 0);

            }
            while(bodyRemaining > 0);
        }
    }
}




