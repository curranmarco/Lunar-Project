#ifndef SOCKETSERVER_H
#define SOCKETSERVER_H

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <bitset>
#include <vector>
#include <algorithm>


class SocketServer {
private:
    int server_fd, client_socket;
    struct sockaddr_in local_addr;
    struct sockaddr_in client_addr;
    socklen_t addrlen;
    u_int32_t isn;
    fd_set master_set, read_set;
    int max_sd;
    std::vector<int> client_sockets;
    std::string lookup[256];

public:
    explicit SocketServer(int port);
    bool startListening();
    bool handleConnections();
    void closeConnection(int client_socket);
    ~SocketServer();
    void sendData(int client_socket, std::string packet);
    std::string receiveData(int client_socket);
    std::string AckPacket(u_int32_t snc, u_int32_t size, std::string flag);
    std::bitset<16> headerChecksum(std::string header);
    void handshake(int client_socket, std::string syn);
};

#endif