#include <iostream>
#include <unistd.h>
#include <thread>
#include <algorithm>
#include <vector>
#include <optional>
#include <memory>
#include "../lib/socketUtil.h"
#include <mutex>

struct AcceptedSocket
{
    int acceptedSocketFD;
    sockaddr_in addr;
    int error;
    bool accepted;
    std::string name;
};

// Defines the max connections the server can take in
constexpr size_t MAX_CONNECTIONS = 10;

// Define the max message size the server/client can take or send
constexpr size_t MAX_BUFFER_SIZE = 1024;

std::vector<std::unique_ptr<AcceptedSocket>> acceptedSockets;

std::mutex acceptedSocketsMutex;

void receiveAndPrintIncomingDataOnSeperateThread(const int socketFD);

void receiveAndPrintIncomingData(int serverSocketFD);

std::unique_ptr<AcceptedSocket> acceptIncomingConnection(int serverSocketFD);

void startAcceptingIncomingConnections(int serverSocketFD);

void sendReceiveMessageToTheOtherClients(const char *buffer, int serverSocketFD);

void getRidOfFDFromAcceptedSockets(int serverSocketFD);

bool specialCommands(std::string_view j, const int serverSocketFD);

int main()
{
    acceptedSockets.reserve(MAX_CONNECTIONS);
    for (int i = 0; i < MAX_CONNECTIONS; i++)
        acceptedSockets.push_back(nullptr);

    int serverSocketFD{createTCPIPv4Socket()};
    std::unique_ptr<sockaddr_in> address = createIPv4Address("", 2000);

    int result{bind(serverSocketFD, (const sockaddr *)address.get(), sizeof *address)};

    if (result == 0)
        std::cout << "Server was bound successfully!\n"
                  << std::flush;

    int listenResult{listen(serverSocketFD, 10)};

    startAcceptingIncomingConnections(serverSocketFD);

    shutdown(serverSocketFD, SHUT_RDWR);

    return 0;
}

bool specialCommands(std::string_view j, const int serverSocketFD)
{
    bool foundCommand{false};

    std::string msg(j.substr(j.find(':') + 2));

    auto tokenize = [&msg]() -> std::vector<std::string>
    {
        std::vector<std::string> returner;
        std::string curr;
        for (auto c : msg)
        {
            if (c == ' ' && !curr.empty())
            {
                returner.push_back(curr);
                curr.clear();
            }
            else
            {
                curr += c;
            }
        }
        if (!curr.empty())
            returner.push_back(curr);
        return returner;
    };

    std::vector<std::string> tokens = tokenize();
    if (tokens.empty())
        return false;

    if (tokens[0] == "/all")
    {
        foundCommand = true;
        std::lock_guard<std::mutex> lock(acceptedSocketsMutex);
        std::string sender = "Connected users: \n";
        for (const auto &curr : acceptedSockets)
            if (curr && !curr.get()->name.empty())
                sender += curr.get()->name +
                          (curr.get()->acceptedSocketFD == serverSocketFD ? " (You) " : "") +
                          "\n";

        if (send(serverSocketFD, sender.c_str(), sender.length(), 0) <= 0)
            std::cerr << "Failed to send all users!\n";
    }
    else if (tokens[0] == "/msg")
    {
        foundCommand = true;
        if (tokens.size() < 3)
        {
            const std::string errorMsg{"/help for correct syntax of sending message\n"};
            if (send(serverSocketFD, errorMsg.c_str(), errorMsg.length(), 0) <= 0)
                std::cerr << "Could not send to user!\n";
        }
        std::lock_guard<std::mutex> lock(acceptedSocketsMutex);
        std::string userToSendTo{tokens[1]};
        std::string msg = " (PM) ";
        for (size_t i = 2; i < tokens.size(); i++)
            msg.append(" " + tokens[i]);

        auto user = std::find_if(acceptedSockets.begin(),
                                 acceptedSockets.end(),
                                 [&userToSendTo](auto &curr)
                                 {
                                     return curr && curr.get()->name == userToSendTo;
                                 });
        if (user == acceptedSockets.end())
        {
            std::string failedToFindUser = "Could not find user with name : " + userToSendTo + "!\n";
            if (send(serverSocketFD, failedToFindUser.c_str(), failedToFindUser.length(), 0) <= 0)
                std::cerr << "Could not send error msg!\n";
        }
        else if (send(user->get()->acceptedSocketFD, msg.c_str(), msg.length(), 0) <= 0)
            std::cerr << "Failed to send all private msg!\n";
    }

    return foundCommand;
}

void getRidOfFDFromAcceptedSockets(int serverSocketFD)
{
    std::lock_guard<std::mutex> lock(acceptedSocketsMutex);
    if (auto it = std::find_if(acceptedSockets.begin(),
                               acceptedSockets.end(),
                               [serverSocketFD](auto &curr)
                               {
                                   return curr && curr.get()->acceptedSocketFD == serverSocketFD;
                               });
        it != acceptedSockets.end())
    {
        std::cout << "Removing user \"" << it->get()->name << "\"" << std::endl;
        it->reset();
    }
    else
    {
        std::cerr << "Could not find server socket to get rid of!\n";
    }
}

void receiveAndPrintIncomingData(int serverSocketFD)
{
    bool userNameFound{false};

    char buffer[MAX_BUFFER_SIZE]{};
    while (true)
    {
        ssize_t amtRecieved = recv(serverSocketFD, buffer, MAX_BUFFER_SIZE, 0);

        if (amtRecieved > 0)
        {
            std::cout << buffer << std::endl;

            if (!specialCommands(std::string(buffer), serverSocketFD))
            {
                sendReceiveMessageToTheOtherClients(buffer, serverSocketFD);
            }
            if (!userNameFound)
            {
                std::lock_guard<std::mutex> lock(acceptedSocketsMutex);

                auto it = std::find_if(acceptedSockets.begin(),
                                       acceptedSockets.end(),
                                       [serverSocketFD](auto &curr)
                                       {
                                           return curr.get()->acceptedSocketFD == serverSocketFD;
                                       });

                std::string name(buffer);
                if (it == acceptedSockets.end())
                {
                    std::cerr << "Could not initiazlie user name!\n";
                }

                it->get()->name = name.substr(0, name.find(':'));
                userNameFound = true;
            }
        }
        else
        {
            std::cout << "Client stopped connection!\n";
            break;
        }

        std::fill(std::begin(buffer), std::end(buffer), 0);
    }
    close(serverSocketFD);
    getRidOfFDFromAcceptedSockets(serverSocketFD);
}

std::unique_ptr<AcceptedSocket> acceptIncomingConnection(int serverSocketFD)
{
    sockaddr_in clientAddy;
    socklen_t clientAddySize = sizeof(sockaddr_in);
    int clientSocketFD = accept(serverSocketFD, (sockaddr *)&clientAddy, &clientAddySize);

    std::unique_ptr<AcceptedSocket> returner = std::make_unique<AcceptedSocket>();

    // AcceptedSocket *returner = new AcceptedSocket;

    returner->addr = clientAddy;
    returner->acceptedSocketFD = clientSocketFD;
    returner->accepted = clientSocketFD >= 0;

    if (!returner->accepted)
    {
        returner->error = clientSocketFD;
    }

    return returner;
}

void startAcceptingIncomingConnections(int serverSocketFD)
{
    while (true)
    {
        std::unique_ptr<AcceptedSocket> accepted = acceptIncomingConnection(serverSocketFD);

        std::lock_guard<std::mutex> lock(acceptedSocketsMutex);
        auto spot = std::find_if(acceptedSockets.begin(),
                                 acceptedSockets.end(),
                                 [](auto &curr)
                                 {
                                     return !curr;
                                 });

        if (spot == acceptedSockets.end())
        {
            std::cerr << "Could not find an open connection!\n";
            close(accepted->acceptedSocketFD);
            continue;
        }

        *spot = std::move(accepted);
        receiveAndPrintIncomingDataOnSeperateThread(spot->get()->acceptedSocketFD);
    }
}

void receiveAndPrintIncomingDataOnSeperateThread(const int socketFD)
{
    std::thread t1(receiveAndPrintIncomingData, socketFD);
    t1.detach();
}

void sendReceiveMessageToTheOtherClients(const char *buffer, int serverSocketFD)
{
    std::lock_guard<std::mutex> lock(acceptedSocketsMutex);
    for (int i = 0; i < acceptedSockets.size(); i++)
        if (acceptedSockets[i] && acceptedSockets[i].get()->acceptedSocketFD != serverSocketFD)
        {
            if (send(acceptedSockets[i].get()->acceptedSocketFD, buffer, strlen(buffer), 0) == -1)
                std::cerr << "Could not send message to one of the users!\n";
        }
}
