#ifndef SOCKETSERVER_H
#define SOCKETSERVER_H

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <bitset>

class SocketServer {
private:
    int server_fd, client_socket;
    struct sockaddr_in local_addr;
    struct sockaddr_in client_addr;
    int addrlen;
    u_int32_t isn;

public:
    explicit SocketServer(int port);
    bool startListening();
    bool acceptClient();
    void closeConnection();
    ~SocketServer();
    void sendData();
    void receiveData();
    std::string SynAckPacket(u_int32_t isnc);
};

#endif