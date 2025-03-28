#ifndef SOCKETCLIENT_H
#define SOCKETCLIENT_H

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <bitset>
#include <cmath>
#include <netinet/in.h> 
class SocketClient {
private:
    int sock;
    struct sockaddr_in server_addr;
    struct sockaddr_in local_addr;
    u_int32_t isn;

public:
    explicit SocketClient(const std::string& server_ip, int port);
    bool connectToServer(int source_port = 0);
    void closeConnection();
    void sendData();
    void receiveData();
    std::string SynPacket();
    std::string AckPacket(u_int32_t isns);
    std::string FinPacket();
    std::string DataPacket(u_int32_t seq_num, u_int32_t ack_num, const std::string& payload);
    ~SocketClient();
};

#endif
