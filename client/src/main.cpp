#include <iostream>
#include <string>

#include <ncurses.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <csignal>

#include "../include/ChatClient.h"

/*
 * Any sigpipes we get are probably from disconnected server. Let's catch them here.
 */
void SIGPIPE_HANDLER(int sig)
{
    std::cout << std::endl << "Remote has hung up. Disconnecting." << std::endl;
    system("reset -Q");
    std::cout << std::endl << "Terminal reset called to fix any readline blocking issues." << std::endl;
    exit(0);
}

int main(int argc, char** argv) {

    if(argc != 3)
    {
        std::cout << "Usage:\n ./client host port\n";
        exit(1);
    }

    signal(SIGPIPE, SIGPIPE_HANDLER);

    std::cout << "Chat client v1.0" << std::endl;

    auto handle = readline("User handle: ");
    std::cout << "Handle set to: " << handle << std::endl << std::flush;

    ChatClient client(handle);

    client.Connect(argv[1], argv[2]);

    std::string toSend;

    while(true)
    {
        toSend = readline((std::string(handle) + std::string("> ")).c_str());

        // Obviously this could be better :)
        if(toSend == "\\quit")
        {
            client.Disconnect();
            break;
        }

        client.SendMessage(toSend);
    }

    return 0;
}