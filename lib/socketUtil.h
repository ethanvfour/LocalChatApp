#ifndef SOCKET_HEADER
#define SOCKET_HEADER

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <memory>

//https://www.youtube.com/watch?v=KEiur5aZnIM 24 min

int createTCPIPv4Socket();

std::unique_ptr<sockaddr_in> createIPv4Address(const char * ip, int portNum);

#endif