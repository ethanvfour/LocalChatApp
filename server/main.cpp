#include "serverHelper/server.h"

int main()
{
    acceptedSockets.reserve(MAX_CONNECTIONS);
    
    int serverSocketFD{createTCPIPv4Socket()};

    if(serverSocketFD <= 0)
    {
        std::cerr << "Could not create socket!\n";
        return 1;
    }

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
