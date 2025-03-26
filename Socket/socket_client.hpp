#ifndef SOCKETCLIENT_H
#define SOCKETCLIENT_H

#include <iostream>
#include <string>

#if defined(_WIN32) || defined(_WIN64)
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "Ws2_32.lib")
    typedef SOCKET SocketType;
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    typedef int SocketType;
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
#endif

class SocketClient {
private:
    SocketType sock;
    struct sockaddr_in server_addr;

#if defined(_WIN32) || defined(_WIN64)
    WSADATA wsa_data;
#endif

public:
    explicit SocketClient(const std::string& server_ip, int port);
    bool connectToServer();
    void closeConnection();
    ~SocketClient();
    int getSocket() const { return sock; }

};

#endif // SOCKETCLIENT_H
