#ifndef SOCKETSERVER_H
#define SOCKETSERVER_H

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

class SocketServer {
private:
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen;

public:
    explicit SocketServer(int port);
    bool startListening();
    bool acceptClient();
    void closeConnection();
    ~SocketServer();
};

#endif