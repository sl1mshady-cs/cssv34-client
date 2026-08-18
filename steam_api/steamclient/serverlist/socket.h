#ifndef _SOCKET_H
#define _SOCKET_H

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

typedef struct WSAData {
    unsigned short          wVersion;
    void*                   _placeholder;
} WSADATA;

#define WSAStartup()
#define WSACleanup()
#define closesocket close
#endif

#endif // _SOCKET_H