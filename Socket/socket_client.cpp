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
int SocketClient::getSocket() const {
    return sock;
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


/* ALL PACKET RELATED FUNCTIONS HAVE BEEN MOVED TO PACKET DIRECTORY*/