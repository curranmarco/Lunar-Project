#ifndef SOCKETCLIENT_H
#define SOCKETCLIENT_H

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <bitset>
#include <cmath>
#include <netinet/in.h> 
#include "../Packet/packet_functions.hpp" 
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
   
    ~SocketClient();
    bool SendPacket(int socket_fd, const std::string& packet);
    int getSocket() const;

};

#endif

/* ALL PACKET RELATED FUNCTIONS HAVE BEEN MOVED TO PACKET DIRECTORY*/