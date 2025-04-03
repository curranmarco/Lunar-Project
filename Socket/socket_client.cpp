#include "socket_client.hpp"

SocketClient::SocketClient(const std::string& server_ip, int port) {
    // Create the client socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        std::cerr << "Socket creation failed.\n";
    }

    // Set up the server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);                 // host to network port translation

    // Convert IP address from string to binary
    if (inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address.\n";
    }
}

bool SocketClient::connectToServer(int source_port) {           // Optionally specify the source port or let the OS decide
    std::cout << "Connecting to the server...\n";

    if (source_port > 0) {
        local_addr.sin_family = AF_INET;
        local_addr.sin_addr.s_addr = INADDR_ANY;  // Bind to any available local interface
        local_addr.sin_port = htons(source_port);

        if (bind(server_socket, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
            std::cerr << "Failed to bind source port " << source_port << ".\n";
            return false;
        }
        std::cout << "Bound to source port: " << source_port << "\n";
    }

    if (connect(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connection failed.\n";
        return false;
    }
    std::cout << "Connected to the server.\n";
    return true;
}

int SocketClient::getServerSocket() {
    return SocketClient::server_socket;
}

void SocketClient::closeConnection() {
    close(server_socket);
}

SocketClient::~SocketClient() {
    closeConnection();
}

/**********************************************************************************/
/**********************************************************************************/
/*                                                                                */
/*  The following functions are used to send and receive data to the server.      */
/*                                                                                */
/*  For the purpose of simplifying the code base and ensuring we can reuse code   */
/*  blocks throughout the project, these functions will run continuously until    */
/*  they receive a header with a stop instruction.                                */
/*                                                                                */
/**********************************************************************************/
/**********************************************************************************/

void SocketClient::sendData(int client_socket, std::string packet) {
    int bytes_sent = send(server_socket, packet.c_str(), packet.size(), 0);
    if (bytes_sent < 0) {
        std::cerr << "Failed to send data.\n";
    }
    std::cout << "Sent: " << packet << "\n";
    sleep(1);
}

std::string SocketClient::receiveData() {
    std::string packet(1024, '\0');
        int bytes_read = recv(server_socket, &packet[0], packet.size(), 0);

        if (bytes_read <= 0) {
            std::cerr << "Reading failed.\n";
            SocketClient::closeConnection();
            std::cout << "Socket Closed\n";
            return "";
        }

        else {
            packet.resize(bytes_read);
            return packet;
        } 
}

std::string SocketClient::SynPacket() {
    std::string syn;
    std::bitset<5 * 32> header;

    //Source Port
    std::bitset<16> source(ntohs(local_addr.sin_port));
    syn += source.to_string();

    // Destination Port
    std::bitset<16> dest(ntohs(server_addr.sin_port));
    syn += dest.to_string();

    // Sequence Number
    int isn = (rand() % ((int)exp2(32) - 1));
    std::bitset<32> isnb(isn);
    syn += isnb.to_string();

    // Ack Number
    std::bitset<32> ack_num(0);
    syn += ack_num.to_string();

    // Data Offset
    std::bitset<4> offset(5);                               // 5 Words in header
    syn += offset.to_string();

    // Reserved Space
    std::bitset<3> res(0);
    syn += res.to_string();

    // Flags
    std::bitset<9> flag(6);                                // 6 = 00000110 which is the flag for SYN and SNS
    syn += flag.to_string();

    // Window Size
    std::bitset<16> window(24);                            // 6 words
    syn += window.to_string();

    // Urgent Pointer (Not used so just 0)
    std::bitset<16> urg(0);

    // Checksum (Take as zero for sum, hence 2 urg strings concatenated)
    std::bitset<16> check = headerChecksum(syn + urg.to_string() + urg.to_string()); 

    syn += check.to_string() + urg.to_string();

    return syn;
}

std::string SocketClient::AckPacket(u_int32_t sns, u_int32_t size) {
    std::string ack;

    // Source Port
    std::bitset<16> source(ntohs(local_addr.sin_port));
    ack += source.to_string();

    // Destination Port
    std::bitset<16> dest(ntohs(server_addr.sin_port));
    ack += dest.to_string();

    // Sequence Number
    std::bitset<32> snb(sns + size);
    ack += snb.to_string();

    // Acknowledgement Number
    std::bitset<32> ack_num(sns);
    ack += ack_num.to_string();

    // Data Offset
    std::bitset<4> offset(5);                               // 5 Words in header
    ack += offset.to_string();

    // Reserved Space
    std::bitset<3> res(0);
    ack += res.to_string();

    // Flags
    std::bitset<9> flag(16);                                // 16 = 00010000 which is the flag for ACK
    ack += flag.to_string();

    // Window Size
    std::bitset<16> window(24);                            // 6 words
    ack += window.to_string();

    // Urgent Pointer (Not used so just 0)
    std::bitset<16> urg(0);

    std::cout << ack + urg.to_string() + urg.to_string();

    // Checksum (Take as zero for sum, hence 2 urg strings concatenated)
    std::bitset<16> check = headerChecksum(ack + urg.to_string() + urg.to_string());  
    ack += check.to_string() + urg.to_string();

    return ack;
}

std::string SocketClient::FinPacket(u_int32_t sns, u_int32_t size) {
    std::string fin;

    // Source Port
    std::bitset<16> source(ntohs(local_addr.sin_port));
    fin += source.to_string();

    // Destination Port
    std::bitset<16> dest(ntohs(server_addr.sin_port));
    fin += dest.to_string();

    // Sequence Number
    std::bitset<32> snb(sns + size);
    fin += snb.to_string();

    std::bitset<32> ack_num(sns);
    fin += ack_num.to_string();

    // Data Offset
    std::bitset<4> offset(5);                               // 5 Words in header
    fin += offset.to_string();

    // Reserved Space
    std::bitset<3> res(0);
    fin += res.to_string();

    std::bitset<9> flag(1);                                // 1 = 00000001 which is the flag for FIN
    fin += flag.to_string();

    // Window Size
    std::bitset<16> window(24);                            // 6 words
    fin += window.to_string();

    // Urgent Pointer (Not used so just 0)
    std::bitset<16> urg(0);

    // Checksum (Take as zero for sum, hence 2 urg strings concatenated)
    std::bitset<16> check = headerChecksum(fin + urg.to_string() + urg.to_string()); 

    fin += check.to_string() + urg.to_string();

    return fin;
}

std::string SocketClient::SensorPacket(u_int32_t sns, u_int32_t size, std::string data) {
    std::string sensor;

    // Source Port
    std::bitset<16> source(ntohs(local_addr.sin_port));
    sensor += source.to_string();

    // Destination Port
    std::bitset<16> dest(ntohs(server_addr.sin_port));
    sensor += dest.to_string();

    // Sequence Number
    std::bitset<32> snb(sns + size);
    sensor += snb.to_string();

    // Acknowledgement Number
    std::bitset<32> ack_num(sns);
    sensor += ack_num.to_string();

    // Data Offset
    std::bitset<4> offset(5);                               // 5 Words in header
    sensor += offset.to_string();

    // Reserved Space
    std::bitset<3> res(0);
    sensor += res.to_string();

    // Flags
    std::bitset<9> flag(4);                                // 4 = 00000100 which is the flag for ACK
    sensor += flag.to_string();

    // Window Size
    std::bitset<16> window(24);                            // 6 words
    sensor += window.to_string();

    // Urgent Pointer (Not used so just 0)
    std::bitset<16> urg(0);

    std::cout << sensor + urg.to_string() + urg.to_string();

    // Checksum (Take as zero for sum, hence 2 urg strings concatenated)
    std::bitset<16> check = headerChecksum(sensor + urg.to_string() + urg.to_string());  
    sensor += check.to_string() + urg.to_string();

    return sensor;
}

std::bitset<16> SocketClient::headerChecksum(std::string header) {
    // Ensure header length is a multiple of 16 bits.
    if (header.size() % 16 != 0) {
        std::cout << "Header size is: " << header.size() << std::endl;
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