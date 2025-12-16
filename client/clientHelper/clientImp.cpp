#include "client.h"

void startListeningAndPrintMessagesOnNewThread(int socketFD)
{
    // int * j = new int(socketFD);
    // pthread_t id;
    // pthread_create(&id, NULL, listenAndPrint, j);

    std::thread t(listenAndPrint, socketFD);
    t.detach();
}

void listenAndPrint(int socketFDArg)
{
    int socketFD = socketFDArg;
    char buffer[1024] = {0};
    while (true)
    {
        ssize_t amtRecieved = recv(socketFD, buffer, 1024, 0);
        if (amtRecieved > 0)
        {
            std::cout << buffer << std::endl;
        }
        else
        {
            std::cout << "Server stopped connection!\n";
            break;
        }

        std::fill(std::begin(buffer), std::end(buffer), 0);
    }
    close(socketFD);
}