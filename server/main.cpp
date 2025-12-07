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
};

const size_t MAX_CONNECTIONS = 10;
std::vector<std::unique_ptr<AcceptedSocket>> acceptedSockets(MAX_CONNECTIONS, nullptr);

std::mutex acceptedSocketsMutex;

void receiveAndPrintIncomingDataOnSeperateThread(const int socketFD);

void receiveAndPrintIncomingData(int socketFd);

std::unique_ptr<AcceptedSocket> acceptIncomingConnection(int serverSocketFD);

void startAcceptingIncomingConnections(int serverSocketFD);

void sendReceiveMessageToTheOtherClients(const char *buffer, int serverSocketFD);

void getRidOfFDFromAcceptedSockets(int serverSocketFD);

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
        it->reset();
    }
    else
    {
        std::cerr << "Could not find server socket to get rid of!\n";
    }
}

void receiveAndPrintIncomingData(int socketFd)
{
    int serverSocketFD = socketFd;

    char buffer[1024] = {0};
    while (true)
    {
        ssize_t amtRecieved = recv(serverSocketFD, buffer, 1024, 0);
        if (amtRecieved > 0)
        {
            std::cout << buffer << std::endl;

            sendReceiveMessageToTheOtherClients(buffer, serverSocketFD);
        }

        if (amtRecieved == 0)
        {
            std::cout << "Client stopped connection!\n";
            break;
        }

        if (amtRecieved < 0)
            break;

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
        auto spot = std::find_if(acceptedSockets.begin(), acceptedSockets.end(), [](auto &curr)
                                 { return !curr; });
        if (spot == acceptedSockets.end())
        {
            std::cerr << "Could not find an open connection!\n";
            close(accepted->acceptedSocketFD);
            continue;
        }

        *spot = std::move(accepted);
        receiveAndPrintIncomingDataOnSeperateThread((*spot).get()->acceptedSocketFD);
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

int main()
{
    int serverSocketFD = createTCPIPv4Socket();
    sockaddr_in *address = createIPv4Address("", 2000);

    int result = bind(serverSocketFD, (const sockaddr *)address, sizeof(*address));

    if (result == 0)
        std::cout << "Server was bound successfully!\n"
                  << std::flush;

    int listenResult = listen(serverSocketFD, 10);

    startAcceptingIncomingConnections(serverSocketFD);

    shutdown(serverSocketFD, SHUT_RDWR);

    return 0;
}