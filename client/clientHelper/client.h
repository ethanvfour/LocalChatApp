#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <thread>
#include <unistd.h>
#include <algorithm>
#include <sys/socket.h>

void startListeningAndPrintMessagesOnNewThread(int socketFD);

void listenAndPrint(int socketFDArg);

#endif