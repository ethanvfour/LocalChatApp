#include "serverHelper/server.h"

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
