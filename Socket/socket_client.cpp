#include "socket_client.hpp"

SocketClient::SocketClient(const std::string& server_ip, int port) {
    // Create the client socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
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

        if (bind(sock, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
            std::cerr << "Failed to bind source port " << source_port << ".\n";
            return false;
        }
        std::cout << "Bound to source port: " << source_port << "\n";
    }

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connection failed.\n";
        return false;
    }
    std::cout << "Connected to the server.\n";
    return true;
}

void SocketClient::closeConnection() {
    close(sock);
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

void SocketClient::sendData() {
    std::bitset<16> binary(ntohs(server_addr.sin_port));

    std::cout << "The destination port is: " << ntohs(server_addr.sin_port) << std::endl;           // network to host port translation
    std::cout << "Which is " << binary.to_string() << " in binary" << std::endl;
    std::cout << "The source port is:       " << ntohs(local_addr.sin_port) << std::endl;
    
}

void SocketClient::receiveData() {

}

std::string SocketClient::SynPacket() {
    std::string syn;
    std::bitset<5 * 32> header;

    //Source Port
    std::bitset<16> source(8080);
    syn += source.to_string();

    // Destination Port
    std::bitset<16> dest(33634);
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
    std::bitset<9> flag(2);                                // 2 = 00000010 which is the flag for SYN
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
    ack += source.to_string() + " ";

    // Destination Port
    std::bitset<16> dest(ntohs(server_addr.sin_port));
    ack += dest.to_string() + " ";

    // Sequence Number
    std::bitset<32> snb(sns + size);
    ack += snb.to_string() + " ";

    // Acknowledgement Number
    std::bitset<32> ack_num(sns);
    ack += ack_num.to_string() + " ";

    // Data Offset
    std::bitset<4> offset(5);                               // 5 Words in header
    ack += offset.to_string();

    // Reserved Space
    std::bitset<3> res(0);
    ack += res.to_string();

    // Flags
    std::bitset<32> flag(16);                                // 16 = 00010000 which is the flag for ACK
    ack += flag.to_string() + " ";

    // Window Size
    std::bitset<16> window(24);                            // 6 words
    ack += window.to_string();

    // Urgent Pointer (Not used so just 0)
    std::bitset<16> urg(0);

    // Checksum (Take as zero for sum, hence 2 urg strings concatenated)
    std::bitset<16> check = headerChecksum(ack + urg.to_string() + urg.to_string()); 
    ack += check.to_string() + urg.to_string();

    return ack;
}

std::string SocketClient::FinPacket(u_int32_t sns, u_int32_t size) {
    std::string fin;

    // Source Port
    std::bitset<16> source(ntohs(local_addr.sin_port));
    fin += source.to_string() + " ";

    // Destination Port
    std::bitset<16> dest(ntohs(server_addr.sin_port));
    fin += dest.to_string() + " ";

    // Sequence Number
    std::bitset<32> snb(sns + size);
    fin += snb.to_string() + " ";

    std::bitset<32> ack_num(sns);
    fin += ack_num.to_string() + " ";

    // Data Offset
    std::bitset<4> offset(5);                               // 5 Words in header
    fin += offset.to_string();

    // Reserved Space
    std::bitset<3> res(0);
    fin += res.to_string();

    std::bitset<32> flag(1);                                // 1 = 00000001 which is the flag for FIN
    fin += flag.to_string() + " ";

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

std::bitset<16> SocketClient::headerChecksum(std::string header) {
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