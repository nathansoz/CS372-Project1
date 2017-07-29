#include <iostream>
#include <string>

#include "../include/ChatClient.h"

std::string GetUserInput(std::string prompt)
{
    std::string ret;
    std::cout << prompt << ":\n> ";
    getline(std::cin, ret);
    return ret;
}

int main() {
    std::cout << "Chat client v1.0" << std::endl;

    auto handle = GetUserInput("User handle");
    std::cout << "Handle set to: " << handle << std::endl;

    ChatClient client(handle);

    client.Connect(8080);
    std::string test = u8"TEST \xF0\x9F\x98\xBB";
    client.SendMessage(test);

    std::string toSend;

    while(true)
    {
        toSend = GetUserInput("message");

        if(toSend == "\\quit")
        {
            break;
        }

        client.SendMessage(toSend);
    }

    client.Disconnect();

    return 0;
}