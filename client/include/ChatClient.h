//
// Created by nathan on 7/22/17.
//

#ifndef CLIENT_CHATCLIENT_H
#define CLIENT_CHATCLIENT_H

#include <thread>
#include <queue>
#include <mutex>
#include <string>
#include <atomic>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>


class ChatClient
{
public:
    ChatClient(std::string handle);
    ~ChatClient();
    void Connect(char* host, char* port);
    void Disconnect();
    void SendMessage(std::string message);
    bool Closed();
private:
    void SendLoop();
    void ReceiveLoop();
    void DisconnectLoop();
    void SafeDisconnect();

    std::mutex _sendQueueLock;
    std::queue<std::string> _sendQueue;

    int _socketFileDescriptor = -1;
    std::thread _sendWorker;
    std::thread _receiveWorker;
    std::thread _disconnectWorker;

    std::atomic<bool> _shouldDisconnect;
    std::atomic<bool> _disconnecting;
    std::string _userHandle;

};

inline void ChatClient::SendMessage(std::string message)
{
    _sendQueueLock.lock();
    _sendQueue.push(message);
    _sendQueueLock.unlock();
}

inline bool ChatClient::Closed()
{
    return _shouldDisconnect;
}

#endif //CLIENT_CHATCLIENT_H
