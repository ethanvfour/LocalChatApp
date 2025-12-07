#include <iostream>
#include <thread>
#include <unistd.h>

#include "../lib/socketUtil.h"

void startListeningAndPrintMessagesOnNewThread(int);

void listenAndPrint(int);

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
            std::cout << "Response was: " << buffer << std::endl;
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
    close(socketFD);
}


int main()
{
    int socketFD = createTCPIPv4Socket();
    if (socketFD < 0)
    {
        std::cout << "Failed to create socket!\n";
        return 1;
    }
    
    sockaddr_in *address = createIPv4Address("127.0.0.1", 2000);
    
    // Convert the IP address from text to binary form
    
    int result = connect(socketFD, (const sockaddr *)address, sizeof *address);
    
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
    std::cout<<"Please give name: ";
    std::cin>>name;
    std::cin.clear();
    std::cin.ignore(256, '\n');

    std::string msgConsole;
    std::cout << "send some messages to host (\"exit\" to stop program)" << std::endl;

    startListeningAndPrintMessagesOnNewThread(socketFD);

    while (true)
    {
        getline(std::cin, msgConsole);
        if (msgConsole == "exit")
            break;
        msgConsole = name + ":" + msgConsole;
        ssize_t amountThatWasSent = send(socketFD, msgConsole.c_str(), msgConsole.length(), 0);
        if (amountThatWasSent <= 0)
        {
            std::cout << "Server is done running!\n";
            break;
        }
    }
    close(socketFD);

    delete address;
    return 0;
}
