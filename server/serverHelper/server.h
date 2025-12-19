#ifndef SERVER_H
#define SERVER_H

#include "../lib/socketUtil.h"
#include <iostream>
#include <unistd.h>
#include <thread>
#include <algorithm>
#include <vector>
#include <optional>
#include <memory>
#include <mutex>
#include <atomic>

struct AcceptedSocket
{
    int acceptedSocketFD;
    sockaddr_in addr;
    int error;
    bool accepted;
    std::string name;
};

// Vector that holds all accepted connections
//extern std::vector<std::unique_ptr<AcceptedSocket>> acceptedSockets;

// Defines the max connections the server can take in
constexpr size_t MAX_CONNECTIONS = 10;

extern std::unordered_map<int, std::unique_ptr<AcceptedSocket>> acceptedSockets;

extern std::unordered_map<std::string, int> nameToFd;

extern std::mutex acceptedSocketsMutex;

extern std::atomic<bool> serverDone;

void receiveAndPrintIncomingDataOnSeperateThread(const int socketFD);

void receiveAndPrintIncomingData(int serverSocketFD);

std::unique_ptr<AcceptedSocket> acceptIncomingConnection(int serverSocketFD);

void startAcceptingIncomingConnections(int serverSocketFD);

void sendReceiveMessageToTheOtherClients(const char *buffer, int serverSocketFD);

void getRidOfFDFromAcceptedSockets(int serverSocketFD);

bool handleCommand(std::string_view j, const int clientSockedFD);

void serverStopper(int serverSocketFD);

#endif