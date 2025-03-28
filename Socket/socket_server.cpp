#include "socket_server.hpp"

SocketServer::SocketServer(int port) {
    addrlen = sizeof(local_addr);

    // Create the server socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Socket creation failed.\n";
    }

    // Set up the server address structure
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = htons(port);

    // Bind the socket
    if (bind(server_fd, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
        std::cerr << "Bind failed.\n";
    }
}

bool SocketServer::startListening() {
    if (listen(server_fd, 1) < 0) {
        std::cerr << "Listen failed.\n";
        return false;
    }
    std::cout << "Waiting for a connection...\n";
    return true;
}

bool SocketServer::acceptClient() {
    client_socket = accept(server_fd, (struct sockaddr*)&client_addr, (socklen_t*)&addrlen);
    if (client_socket < 0) {
        std::cerr << "Accept failed.\n";
        return false;
    }
    std::cout << "Connection established.\n";
    return true;
}

void SocketServer::closeConnection() {
    close(client_socket);
    close(server_fd);
}

SocketServer::~SocketServer() {
    closeConnection();
}

void SocketServer::sendData() {
    std::bitset<16> binary(ntohs(client_addr.sin_port));

    std::cout << "The destination port is: " << ntohs(client_addr.sin_port) << std::endl;           // network to host port translation
    std::cout << "Which is " << binary.to_string() << " in binary" << std::endl;
    std::cout << "The source port is:       " << ntohs(local_addr.sin_port) << std::endl;
    
}

/*void SocketServer::receiveData() {

}*/ 
std::string SocketServer::DataPacket(u_int32_t seq_num, u_int32_t ack_num, const std::string& payload) {
    std::string data_packet;

    std::bitset<16> source(ntohs(local_addr.sin_port));
    data_packet += source.to_string() + " ";

    std::bitset<16> dest(ntohs(client_addr.sin_port));
    data_packet += dest.to_string() + " ";

    std::bitset<32> seq(seq_num);
    data_packet += seq.to_string() + " ";

    std::bitset<32> ack(ack_num);
    data_packet += ack.to_string() + " ";

    std::bitset<32> flags(16);  // ACK = 0x10 = 00010000
    data_packet += flags.to_string() + " ";

    std::bitset<32> payload_size(payload.size());
    data_packet += payload_size.to_string() + " ";

    // Payload is raw string data, added as-is (no binary encoding here)
    data_packet += payload;

    return data_packet;
}

std::string SocketServer::SynAckPacket(u_int32_t isnc) {
    std::string syn_ack;

    std::bitset<16> source(ntohs(local_addr.sin_port));
    syn_ack += source.to_string() + " ";

    std::bitset<16> dest(ntohs(client_addr.sin_port));
    syn_ack += dest.to_string() + " ";

    std::bitset<32> isnb(isn);
    syn_ack += isnb.to_string() + " ";

    std::bitset<32> ack_num(++isnc);
    syn_ack += ack_num.to_string() + " ";

    std::bitset<32> flag(18);                                // 18 = 00010010 which is the flag for SYN-ACK
    syn_ack += flag.to_string() + " ";

    return syn_ack;
}
std::string SocketServer::AckPacket(u_int32_t isns) {
    std::string ack;

    std::bitset<16> source(ntohs(local_addr.sin_port));
    ack += source.to_string() + " ";

    std::bitset<16> dest(ntohs(client_addr.sin_port));
    ack += dest.to_string() + " ";

    std::bitset<32> isnb(isns);             // fixed here
    ack += isnb.to_string() + " ";

    std::bitset<32> ack_num(isns + 1);      // fixed here
    ack += ack_num.to_string() + " ";

    std::bitset<32> flag(16);               // 16 = ACK
    ack += flag.to_string() + " ";

    return ack;
}


std::string SocketServer::FinPacket() {
    std::string syn;

    std::bitset<16> source(ntohs(local_addr.sin_port));
    syn += source.to_string() + " ";

    std::bitset<16> dest(ntohs(client_addr.sin_port));
    syn += dest.to_string() + " ";

    std::bitset<32> isn(rand()); 
    syn += isn.to_string() + " ";

    std::bitset<32> ack_num(0);
    syn += ack_num.to_string() + " ";

    std::bitset<32> flag(2);                                // 2 = 00000010 which is the flag for SYN
    syn += flag.to_string() + " ";

    return syn;
}