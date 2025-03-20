#ifndef SOCKETCLIENT_H
#define SOCKETCLIENT_H

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

class SocketClient {
private:
    int sock;
    struct sockaddr_in server_addr;

public:
    explicit SocketClient(const std::string& server_ip, int port);
    bool connectToServer();
    void closeConnection();
    ~SocketClient();
};

#endif
