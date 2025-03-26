#ifndef SOCKETSERVER_H
#define SOCKETSERVER_H

#include <iostream>
#include <string>

#if defined(_WIN32) || defined(_WIN64)
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "Ws2_32.lib")
    typedef SOCKET SocketType;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    typedef int SocketType;
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
#endif

class SocketServer {
private:
    SocketType server_fd;
    SocketType client_socket;
    struct sockaddr_in address;
    int addrlen;

#if defined(_WIN32) || defined(_WIN64)
    WSADATA wsa_data;
#endif

public:
    explicit SocketServer(int port);
    bool startListening();
    bool acceptClient();
    void closeConnection();
    ~SocketServer();
    int getClientSocket() const { return client_socket; }
};

#endif // SOCKETSERVER_H
