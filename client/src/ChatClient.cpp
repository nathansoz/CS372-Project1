//
// Created by nathan on 7/22/17.
//

#include <iostream>
#include <cstring>
#include <chrono>
#include <thread>
#include <sys/fcntl.h>
#include <memory>

#include "../include/ChatClient.h"
#include "../include/Constants.h"

ChatClient::ChatClient(std::string handle)
{
    _shouldDisconnect = false;
    _disconnecting = false;
    _userHandle = handle;

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
    _receiveWorker.join();

    if(_disconnectWorker.joinable())
    {
        _disconnectWorker.join();
    }

    close(_socketFileDescriptor);
}

void ChatClient::SafeDisconnect()
{
    _disconnecting = true;

    while(!_sendWorker.joinable())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    _sendWorker.join();
    _receiveWorker.join();

    close(_socketFileDescriptor);
}

void ChatClient::DisconnectLoop()
{

    while(!_disconnecting)
    {
        if (_shouldDisconnect) {
            std::cout << std::endl << "Remote has hung up, disconnecting." << std::endl;
            SafeDisconnect();
            std::cout << std::endl << "Now exiting." << std::endl;
            system("reset -Q");
            std::cout << std::endl << "Terminal reset called to fix any readline blocking issues." << std::endl;
            exit(0);
        }
    }
}

void ChatClient::Connect(char* host, char* port)
{
    std::cout << "Connecting on port " << port << std::endl << std::flush;

    struct addrinfo hints, *res, *loop;
    int status;
    char ipstr[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(host, port, &hints, &res)) != 0 || res == nullptr)
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

    fcntl(_socketFileDescriptor, F_SETFL, O_NONBLOCK);

    //did we get to the end of the loop without connecting?
    if(loop == nullptr)
    {
        std::cerr << "Failed to connect to server." << std::endl;
        exit(ERR_CONNECT);
    }

    _sendWorker = std::thread(&ChatClient::SendLoop, this);
    _receiveWorker = std::thread(&ChatClient::ReceiveLoop, this);
    _disconnectWorker = std::thread(&ChatClient::DisconnectLoop, this);
}

void ChatClient::ReceiveLoop()
{
    while(!_disconnecting)
    {
        size_t messageSizeLength = sizeof(uint32_t);
        ssize_t messageSizeRecv = 0;

        auto buffer1 = new unsigned char[messageSizeLength];
        uint32_t messageSize;

        do
        {
            auto toRead = messageSizeLength - messageSizeRecv;
            auto res = recv(_socketFileDescriptor, buffer1 + messageSizeRecv, toRead, 0);
            auto err = errno;

            if(res == -1)
            {
                if(err == EAGAIN || err == EWOULDBLOCK)
                {
                    continue;
                }
                else {

                    std::cout << "Uh oh. recv failure!" << std::endl;
                    exit(1);
                }
            }
            else if(res == 0)
            {
                _shouldDisconnect = true;
                return;
            }
            else
            {
                messageSizeRecv += res;
            }
        }
        while(messageSizeLength - messageSizeRecv > 0 && !_disconnecting);

        if(_disconnecting)
        {
            return;
        }

        memcpy(&messageSize, buffer1, messageSizeLength);
        delete[] buffer1;

        messageSize = ntohl(messageSize);

        if(messageSize <= 0)
            continue;

        size_t messageRemaining = messageSize;
        std::unique_ptr<char[]> buffer = std::unique_ptr<char[]> {new char[messageSize + 1]};

        do
        {
            auto res = recv(_socketFileDescriptor, buffer.get() + (messageSize - messageRemaining), messageRemaining, 0);
            auto err = errno;

            if(res == -1)
            {
                if(err == EAGAIN || err == EWOULDBLOCK)
                {
                    continue;
                }
                else
                {
                    std::cout << "Uh oh. recv failure!" << std::endl;
                    exit(1);
                }
            }
            else
            {

                messageRemaining -= res;
            }
        }
        while(messageRemaining > 0 && !_disconnecting);

        if(_disconnecting)
        {
            return;
        }

        buffer[messageSize] = '\0';
        std::string toPrint(buffer.get());

        // Wipe out the current line, overwrite, and reprompt.
        std::cout.flush();
        std::cout << "\33[2K" << '\r' << toPrint << std::endl;
        std::cout.flush();
        std::cout << _userHandle << "> ";
        std::cout.flush();
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




