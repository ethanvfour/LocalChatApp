#include "clientHelper/client.h"
#include "../lib/socketUtil.h"

constexpr std::string_view HELP_MESSAGE =
    "/all - Get list of all users connected\n"
    "/msg <user> <msg> - Send a private message to a specific user\n";

int main()
{
    int socketFD{createTCPIPv4Socket()};
    if (socketFD <= 0)
    {
        std::cerr << "Failed to create socket!\n";
        return 1;
    }

    std::unique_ptr<sockaddr_in> address = createIPv4Address("127.0.0.1", 2000);

    // Convert the IP address from text to binary form

    int result{connect(socketFD, (const sockaddr *)address.get(), sizeof *address)};

    if (result == 0)
    {
        std::cout << "Connecttion good!\n";
    }
    else
    {
        std::cerr << "Connection failed!\n";
        return 1;
    }
    std::string name;
    std::cout << "Please give name: ";
    std::cin >> name;
    std::cin.clear();
    std::cin.ignore(256, '\n');

    std::string msgConsole;
    std::cout << "send some messages to host (\"exit\" to stop program)" << std::endl;

    name.erase(
        std::remove_if(name.begin(), name.end(),
                       [](char c)
                       { return c == '"' || c == ':'; }),
        name.end());

    startListeningAndPrintMessagesOnNewThread(socketFD);

    while (true)
    {
        getline(std::cin, msgConsole);
        if (msgConsole == "exit")
            break;
        if (msgConsole == "/help")
        {
            std::cout << HELP_MESSAGE;
            continue;
        }

        msgConsole = name + " : " + msgConsole;
        std::cout << msgConsole << std::endl;
        ssize_t amountThatWasSent{send(socketFD, msgConsole.c_str(), msgConsole.length(), 0)};
        if (amountThatWasSent <= 0)
        {
            std::cout << "Server is done running!\n";
            break;
        }
    }
    close(socketFD);

    return 0;
}
