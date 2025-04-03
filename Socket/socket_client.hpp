#ifndef SOCKETCLIENT_H
#define SOCKETCLIENT_H

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <bitset>
#include <cmath>
#include <vector>

class SocketClient {
private:
    int server_socket;
    struct sockaddr_in server_addr;
    struct sockaddr_in local_addr;

public:
    explicit SocketClient(const std::string& server_ip, int port);
    bool connectToServer(int source_port = 0);
    void closeConnection();
    void sendData(int client_socket, std::string packet);
    std::string receiveData();
    std::string SynPacket();
    std::string AckPacket(u_int32_t sns, u_int32_t size);
    std::string FinPacket(u_int32_t sns, u_int32_t size);
    std::string SensorPacket(u_int32_t sns, u_int32_t size, std::string data);
    std::bitset<16> headerChecksum(std::string header);
    int getServerSocket();
    ~SocketClient();
};

#endif
