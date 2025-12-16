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

// Defines the max connections the server can take in
constexpr size_t MAX_CONNECTIONS = 10;

// Define the max message size the server/client can take or send
constexpr size_t MAX_BUFFER_SIZE = 1024;

// Vector that holds all accepted connections
extern std::vector<std::unique_ptr<AcceptedSocket>> acceptedSockets;

extern std::mutex acceptedSocketsMutex;

extern std::atomic<bool> serverDone;

void receiveAndPrintIncomingDataOnSeperateThread(const int socketFD);

void receiveAndPrintIncomingData(int serverSocketFD);

std::unique_ptr<AcceptedSocket> acceptIncomingConnection(int serverSocketFD);

void startAcceptingIncomingConnections(int serverSocketFD);

void sendReceiveMessageToTheOtherClients(const char *buffer, int serverSocketFD);

void getRidOfFDFromAcceptedSockets(int serverSocketFD);

bool specialCommands(std::string_view j, const int serverSocketFD);

void serverStopper(int serverSocketFD);

#endif