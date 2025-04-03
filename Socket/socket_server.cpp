#include "socket_server.hpp"
#include "../Packet/packet_functions.hpp"

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
    
    for (int i = 0; i < 4; i++) {
        lookup[i] = "0";
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
                    SocketServer::closeConnection(client_socket);
                    std::cout << "Client disconnected.\n";
                } else {
                    buffer.resize(bytes_read);
                    std::cout << "Received: " << buffer << std::endl;
                    std::cout << "Starting handshake..." << std::endl;
                    SocketServer::handshake(client_socket, buffer);
                    std::cout << "Handshake complete..." << std::endl;
                }
            }
        }
    }
    return 1;
}

void SocketServer::closeConnection(int client_socket) {
    close(client_socket);
    FD_CLR(client_socket, &master_set);
    client_sockets.erase(std::remove(client_sockets.begin(), client_sockets.end(), client_socket), client_sockets.end());
}

SocketServer::~SocketServer() {
    for (int client_socket:client_sockets)
        closeConnection(client_socket);
        std::cout << "Socket Closed\n";
}

void SocketServer::sendData(int client_socket, std::string packet) {
    send(client_socket, packet.c_str(), packet.length(), 0);
}

std::string SocketServer::receiveData(int client_socket) {
    std::string packet(1024, '\0');
        int bytes_read = recv(client_socket, &packet[0], packet.size(), 0);

        if (bytes_read <= 0) {
            std::cerr << "Reading failed.\n";
            SocketServer::closeConnection(client_socket);
            std::cout << "Socket Closed\n";
            return "";
        }

        else {
            packet.resize(bytes_read);
            return packet;
        } 
}

std::string SocketServer::AckPacket(u_int32_t snc, u_int32_t size, std::string flag) {
    std::string syn_ack;

    // Source Port
    std::bitset<16> source(ntohs(local_addr.sin_port));
    syn_ack += source.to_string();

    // Destination Port
    std::bitset<16> dest(ntohs(client_addr.sin_port));
    syn_ack += dest.to_string();

    // Sequence Number
    std::bitset<32> snb(snc + size);
    syn_ack += snb.to_string();

    // Acknowledgement Number
    std::bitset<32> ack_num(snc);
    syn_ack += ack_num.to_string();

    // Data Offset
    std::bitset<4> offset(5);                              // 5 Words in header
    syn_ack += offset.to_string();

    // Reserved Space
    std::bitset<3> res(0);
    syn_ack += res.to_string();

    // Flags
    //std::bitset<9> flag(18);                              // 18 = 000010010 which is the flag for SYN-ACK
    flag[4] = '1';                                          // Add ACK bit
    syn_ack += flag;

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
    // Ensure header length is a multiple of 16 bits.
    if (header.size() % 16 != 0) {
        throw std::invalid_argument("Header length must be a multiple of 16 bits");
    }

    // Split the header into 16-bit words.
    std::vector<std::bitset<16>> words;
    for (size_t i = 0; i < header.size(); i += 16) {
        std::string word_str = header.substr(i, 16);
        // (You might also want to validate that word_str contains only '0' and '1'.)
        words.push_back(std::bitset<16>(word_str));
    }

    // Start with the first 16-bit word.
    std::bitset<16> sum = words[0];

    // Add the remaining words into sum.
    for (size_t j = 1; j < words.size(); j++) {
        std::bitset<16> add_word = words[j];
        bool carry = false;
        // Do bit-by-bit addition.
        for (size_t i = 0; i < 16; i++) {
            bool bit1 = sum[i];
            bool bit2 = add_word[i];
            // Full adder logic for this bit.
            bool result = bit1 ^ bit2 ^ carry;
            carry = (bit1 && bit2) || (bit1 && carry) || (bit2 && carry);
            sum[i] = result;
        }
        // If there's an overflow (carry out), add it back into the sum.
        if (carry) {
            bool c = true;
            for (size_t i = 0; i < 16 && c; i++) {
                bool bit = sum[i];
                sum[i] = bit ^ c;
                c = bit && c;
            }
        }
    }

    // Return one's complement of the sum.
    return ~sum;
}

void SocketServer::handshake(int client_socket, std::string syn) {
    std::string header = syn.substr(0, 128);
    std::string sn_string = syn.substr(32, 32);
    bool is_syn = syn[110] == '1';
    std::cout << "Is this a syn? " << is_syn << std::endl;
    std::cout << "Because the flags are " << syn.substr(104, 9) << std::endl;

    // Add Client to lookup table
    if (is_syn) {
        std::string flag = syn.substr(103, 9);
        flag[7] = '0';                                                  // Remove syn flag bit
        SocketServer::lookup[client_socket] = flag;                     // Add to lookup table
    }
        
    else if (syn[111] == '0' && syn[107] == '0') {                      // If not FIN or ACK (96 + 4 + 3 + 5 / 8)
        std::string flag = syn.substr(103, 9);
        flag[7] = '0';

        int* index;
        for (*index = 0; lookup[*index] != flag; *index++);

        if (*index != client_socket)                                    // Forward packet from home client
            SocketServer::sendData(*index, syn);
        else {                                                          // Forward packet from sensors or actuators
            for (*index = 0; lookup[*index] != "001000000"; *index++);
            SocketServer::sendData(*index, syn);                        // 001000000 Home client flag
        }
    }

    sn_string.erase(std::remove_if(sn_string.begin(), sn_string.end(), [](char c) {
        return c != '0' && c != '1'; // Keep only '0' and '1'
    }), sn_string.end());

    std::bitset<32> sn(sn_string);

    std::string packet = SocketServer::AckPacket(sn.to_ulong(), syn.length(), syn.substr(107, 9));
    Packet pack((uint16_t)local_addr.sin_port, (uint16_t)client_addr.sin_port);

    if(pack.verifyChecksum(syn)) {
        std::cout << "Sent ACK: " << std::endl;
        SocketServer::sendData(client_socket, packet);
        sleep(1);
        std::string ack = SocketServer::receiveData(client_socket);
        std::cout << "Received: " << std::endl;
        header = ack.substr(0, 128);
        
        if (is_syn) {
            if (pack.verifyChecksum(ack));
            else {
                SocketServer::closeConnection(client_socket);
                std::cout << "Checksum Failed\nSocket Closed\n";
            }
        }
        
    }

    else {
        std::cout << "Checksum Failed\nSocket Closed\n"; 
        SocketServer::closeConnection(client_socket);
    }  
}