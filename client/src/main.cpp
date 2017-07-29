#include <iostream>

#include "../include/ChatClient.h"

int main() {
    std::cout << "Chat client v1.0" << std::endl;

    ChatClient client;
    client.Connect(8080);

    return 0;
}