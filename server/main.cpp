#include <iostream>
#include <unistd.h>
#include <thread>
#include <array>
#include "../lib/socketUtil.h";
#include <mutex>

struct AcceptedSocket
{
    int accpetedSocketFD;
    sockaddr_in addr;
    int error;
    bool accepeted;
};

std::array<AcceptedSocket, 10> acceptedSockets;
int acceptedSocketsCount{};
std::mutex acceptedSocketsMutex;

void receiveAndPrintIncomingDataOnSeperateThread(const AcceptedSocket *clientSide);

void receiveAndPrintIncomingData(int socketFd);

AcceptedSocket *acceptIncomingConnection(int serverSocketFD);

void startAcceptingIncomingConnections(int serverSocketFD);

void sendReceiveMessageToTheOtherClients(const char *buffer, int serverSocketFD);

void getRidOfFDFromAcceptedSockets(int serverSocketFD){}

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
}

AcceptedSocket *acceptIncomingConnection(int serverSocketFD)
{
    sockaddr_in clientAddy;
    socklen_t clientAddySize = sizeof(sockaddr_in);
    int clientSocketFD = accept(serverSocketFD, (sockaddr *)&clientAddy, &clientAddySize);

    AcceptedSocket *returner = new AcceptedSocket;

    returner->addr = clientAddy;
    returner->accpetedSocketFD = clientSocketFD;
    returner->accepeted = clientSocketFD > 0;

    if (!returner->accepeted)
    {
        returner->error = clientSocketFD;
    }

    return returner;
}

void startAcceptingIncomingConnections(int serverSocketFD)
{
    while (true)
    {
        AcceptedSocket *clientSide = acceptIncomingConnection(serverSocketFD);
        std::lock_guard<std::mutex> lock(acceptedSocketsMutex);

        acceptedSockets[acceptedSocketsCount++] = *clientSide;
        receiveAndPrintIncomingDataOnSeperateThread(clientSide);
        delete clientSide;
    }
}

void receiveAndPrintIncomingDataOnSeperateThread(const AcceptedSocket *clientSide)
{
    // pthread_t id;
    // int *arg = new int(clientSide->accpetedSocketFD);
    // pthread_create(&id, NULL, receiveAndPrintIncomingData, arg);

    std::thread t1(receiveAndPrintIncomingData, clientSide->accpetedSocketFD);
    t1.detach();
}

void sendReceiveMessageToTheOtherClients(const char *buffer, int serverSocketFD)
{
    std::lock_guard<std::mutex> lock(acceptedSocketsMutex);
    for (int i = 0; i < acceptedSocketsCount; i++)
        if (acceptedSockets[i].accpetedSocketFD != serverSocketFD)
        {
            send(acceptedSockets[i].accpetedSocketFD, buffer, strlen(buffer), 0);
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