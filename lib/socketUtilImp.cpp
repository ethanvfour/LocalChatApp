#include "socketUtil.h"

int createTCPIPv4Socket()
{
    return socket(AF_INET, SOCK_STREAM, 0);
}

sockaddr_in* createIPv4Address(const char * ip, int portNum)
{
    sockaddr_in* address = new sockaddr_in;
    
    // Specify the address family (IPv4)
    address->sin_family = AF_INET;

    // Set the port number (needs to be in network byte order)
    address->sin_port = htons(portNum);


    if(strlen(ip) == 0)
        address->sin_addr.s_addr = INADDR_ANY;
    else
        inet_pton(AF_INET, ip, &address->sin_addr.s_addr);

    // Return the raw pointer (caller is responsible for managing memory)
    return address;
}
