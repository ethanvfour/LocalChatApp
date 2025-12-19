#include "server.h"

// Define the max message size the server/client can take or send
static constexpr size_t MAX_BUFFER_SIZE = 1024;

// Global variable definitions
std::unordered_map<int, std::unique_ptr<AcceptedSocket>> acceptedSockets;
std::mutex acceptedSocketsMutex;
std::atomic<bool> serverDone{false};
std::unordered_map<std::string, int> nameToFd;

bool handleCommand(std::string_view j, const int clientSocketFD)
{
    bool foundCommand{false};
    std::string_view msg{j.substr(j.find(':') + 2)};

    auto tokenize = [&msg]() -> std::vector<std::string>
    {
        std::vector<std::string> returner;
        std::string curr;
        for (auto &c : msg)
        {
            if (c == ' ' && !curr.empty())
            {
                returner.push_back(curr);
                curr.clear();
            }
            else
                curr += c;
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
        std::string sender{*"Connected users: \n"};

        for (const auto &[_, ptr] : acceptedSockets)
            if (ptr && !ptr.get()->name.empty())
                sender += ptr.get()->name +
                          (ptr.get()->acceptedSocketFD == clientSocketFD ? " (You) "
                                                                         : "") +
                          "\n";

        if (send(clientSocketFD, sender.c_str(), sender.length(), 0) <= 0)
            std::cerr << "Failed to send all users!\n";
    }
    else if (tokens[0] == "/msg")
    {
        foundCommand = true;
        if (tokens.size() < 3)
        {
            const std::string errorMsg{"/help for correct syntax of sending message\n"};
            if (send(clientSocketFD, errorMsg.c_str(), errorMsg.length(), 0) <= 0)
                std::cerr << "Could not send to user!\n";
            return true;
        }

        std::lock_guard<std::mutex> lock(acceptedSocketsMutex);
        std::string userToSendTo{tokens[1]};
        std::string messageToSend = " (PM) ";
        for (size_t i = 2; i < tokens.size(); i++)
            messageToSend.append(" " + tokens[i]);

        if (auto it = nameToFd.find(userToSendTo);
            it == nameToFd.end() ||
            acceptedSockets.find(it->second) == acceptedSockets.end() ||
            !acceptedSockets[it->second])
        {
            std::string failedToFindUser = "Could not find user with name : " + userToSendTo + "!\n";
            if (send(clientSocketFD, failedToFindUser.c_str(), failedToFindUser.length(), 0) <= 0)
                std::cerr << "Could not send error msg!\n";
        }
        else if (send(acceptedSockets[it->second]->acceptedSocketFD,
                      messageToSend.c_str(),
                      messageToSend.length(), 0) <= 0)
            std::cerr
                << "Failed to send private msg!\n";
    }
    else if (tokens[0] == "/name")
    {
        foundCommand = true;
        std::lock_guard<std::mutex> lock(acceptedSocketsMutex);
        bool nameChanged = false;
    }

    return foundCommand;
}

void getRidOfFDFromAcceptedSockets(int clientSocketFD)
{
    std::lock_guard<std::mutex> lock(acceptedSocketsMutex);

    if (auto it = acceptedSockets.find(clientSocketFD);
        it == acceptedSockets.end() ||
        !it->second)
    {
        std::cerr << "Could not find server socket to get rid of!\n";
    }
    else
    {
        auto &socket = it->second;
        std::cout << "Removing user \"" << socket->name << "\"" << std::endl;

        nameToFd.erase(socket->name);
        acceptedSockets.erase(it);
    }
}

void receiveAndPrintIncomingData(const int clientSocketFD)
{
    bool userNameFound{false};

    char buffer[MAX_BUFFER_SIZE]{};
    while (true)
    {
        ssize_t amtRecieved = recv(clientSocketFD, buffer, MAX_BUFFER_SIZE, 0);

        if (amtRecieved > 0)
        {
            buffer[amtRecieved] = '\0';
            std::cout << buffer << std::endl;

            if (!handleCommand(std::string(buffer), clientSocketFD))
            {
                sendReceiveMessageToTheOtherClients(buffer, clientSocketFD);
            }
            if (!userNameFound)
            {
                std::lock_guard<std::mutex> lock(acceptedSocketsMutex);
                std::string name = std::string(buffer).substr(0, name.find(':') - 1);

                if (auto it = acceptedSockets.find(clientSocketFD);
                    it != acceptedSockets.end())
                {
                    auto &socket = it->second;
                    socket->name = name;
                    nameToFd[name] = clientSocketFD;
                }
                else
                {
                    std::cerr << "Failed to set username! Name to be intialized : " << name << "\n";
                }
            }
        }
        else
        {
            std::cout << "Client stopped connection!\n";
            break;
        }

        std::fill(std::begin(buffer), std::end(buffer), 0);
    }
    close(clientSocketFD);
    getRidOfFDFromAcceptedSockets(clientSocketFD);
}

std::unique_ptr<AcceptedSocket> acceptIncomingConnection(int serverSocketFD)
{
    sockaddr_in clientAddy;
    socklen_t clientAddySize = sizeof(sockaddr_in);
    int clientSocketFD = accept(serverSocketFD, (sockaddr *)&clientAddy, &clientAddySize);

    if (clientSocketFD == -1)
        return std::unique_ptr<AcceptedSocket>(nullptr);

    std::unique_ptr<AcceptedSocket> returner = std::make_unique<AcceptedSocket>();

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
    std::thread stopper(serverStopper, serverSocketFD);
    stopper.detach();
    while (!serverDone)
    {
        std::unique_ptr<AcceptedSocket> accepted = acceptIncomingConnection(serverSocketFD);

        if (accepted.get() == nullptr)
        {
            if (serverDone)
                break;
            continue;
        }

        std::lock_guard<std::mutex> lock(acceptedSocketsMutex);

        if (acceptedSockets.size() == MAX_CONNECTIONS)
        {
            std::cerr << "Could not find an open connection!\n";
            close(accepted->acceptedSocketFD);
            continue;
        }
        int clientFD{accepted->acceptedSocketFD};
        acceptedSockets[clientFD] = std::move(accepted);
        receiveAndPrintIncomingDataOnSeperateThread(clientFD);
    }
}

void receiveAndPrintIncomingDataOnSeperateThread(const int socketFD)
{
    std::thread t1(receiveAndPrintIncomingData, socketFD);
    t1.detach();
}

void sendReceiveMessageToTheOtherClients(const char *buffer, int clientSockedFD)
{
    std::lock_guard<std::mutex> lock(acceptedSocketsMutex);

    for (const auto &[socketFD, ptr] : acceptedSockets)
        if (ptr && socketFD != clientSockedFD)
            if (send(socketFD, buffer, strlen(buffer), 0) == 0)
                std::cerr << "Could not send message to user : " << ptr->name << "\n";
    
}

void serverStopper(int serverSocketFD)
{
    std::string input;
    while (!serverDone)
    {
        getline(std::cin, input);
        if (input == "stop")
        {
            serverDone = true;
            shutdown(serverSocketFD, SHUT_RDWR);
            close(serverSocketFD);
        }
    }
}
