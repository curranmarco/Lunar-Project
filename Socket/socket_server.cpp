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

    FD_SET(server_fd, &master_set);
    max_sd = server_fd;
}

bool SocketServer::startListening() {
    if (listen(server_fd, 1) < 0) {
        std::cerr << "Listen failed.\n";
        return false;
    }
    std::cout << "Waiting for a connection...\n";
    return true;
}

bool SocketServer::handleConnections() {
    while (true) {
        read_set = master_set;
        int activity = select(max_sd + 1, &read_set, nullptr, nullptr, nullptr);
        if (activity < 0) {
            std::cerr << "Select error.\n";
            break;
        }

        if (FD_ISSET(server_fd, &read_set)) {
            int new_socket = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);
            if (new_socket < 0) {
                std::cerr << "Accept error.\n";
            } else {
                client_sockets.push_back(new_socket);
                FD_SET(new_socket, &master_set);
                max_sd = std::max(max_sd, new_socket);
                std::cout << "New connection accepted.\n";
            }
        }

        for (int client_socket : client_sockets) {
            if (FD_ISSET(client_socket, &read_set)) {       // Is there anything to read?
                std::string buffer(1024, '\0');
                int bytes_read = recv(client_socket, &buffer[0], buffer.size(), 0);
                if (bytes_read <= 0) {
                    close(client_socket);
                    FD_CLR(client_socket, &master_set);
                    client_sockets.erase(::std::remove(client_sockets.begin(), client_sockets.end(), client_socket), client_sockets.end());
                    std::cout << "Client disconnected.\n";
                } else {
                    buffer.resize(bytes_read);
                    std::cout << "Received: " << buffer << std::endl;
                }
            }
        }
    }
    return 1;
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

void SocketServer::receiveData() {

}

std::string SocketServer::SynAckPacket(u_int32_t snc, u_int32_t size) {
    std::string syn_ack;

    // Source Port
    std::bitset<16> source(ntohs(local_addr.sin_port));
    syn_ack += source.to_string() + " ";

    // Destination Port
    std::bitset<16> dest(ntohs(client_addr.sin_port));
    syn_ack += dest.to_string() + " ";

    // Sequence Number
    std::bitset<32> snb(snc + size);
    syn_ack += snb.to_string() + " ";

    // Acknowledgement Number
    std::bitset<32> ack_num(snc);
    syn_ack += ack_num.to_string() + " ";

    // Data Offset
    std::bitset<4> offset(5);                              // 5 Words in header
    syn_ack += offset.to_string();

    // Reserved Space
    std::bitset<3> res(0);
    syn_ack += res.to_string();

    // Flags
    std::bitset<32> flag(18);                              // 18 = 00010010 which is the flag for SYN-ACK
    syn_ack += flag.to_string() + " ";

    // Window Size
    std::bitset<16> window(24);                            // 6 words
    syn_ack += window.to_string();

    // Urgent Pointer (Not used so just 0)
    std::bitset<16> urg(0);

    // Checksum (Take as zero for sum, hence 2 urg strings concatenated)
    std::bitset<16> check = headerChecksum(syn_ack + urg.to_string() + urg.to_string()); 

    syn_ack += check.to_string() + urg.to_string();

    return syn_ack;
}

std::bitset<16> SocketServer::headerChecksum(std::string header) {
    uint16_t count = 0;
    std::bitset<16> sum;
    std::vector<std::string> half_words;
    std::string half_word;

    for (char bit:header) {
        half_word += bit;
        count++;

        if (count == 16) {
            half_words.push_back(half_word);
            count = 0;
            half_word = "";
        }
    }
    std::bitset<16> A(half_words[0]);
    std::bitset<16> B;
    bool cin = 0;

    for (int j = 1; j < half_words.size(); j++) {
        B = std::bitset<16>(half_words[j]);
        for (int i = 0; i < A.size(); i++) {
            bool sum_bit = A[i] ^ B[i] ^ cin;
            bool carry_out = (A[i] & B[i]) | (B[i] & cin) | (A[i] & cin);

            A[i] = sum_bit;  
            cin = carry_out; 
        }
    }

    if (cin) {
        for (int i = 0; i < A.size() && cin; i++) {
            bool sum_bit = A[i] ^ cin;
            cin = A[i] & cin;
            A[i] = sum_bit;
        }
    }

    return ~A;
}

void SocketServer::handshake(std::string syn) {

}