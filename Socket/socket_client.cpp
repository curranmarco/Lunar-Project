#include "socket_client.hpp"

SocketClient::SocketClient(const std::string& server_ip, int port) {
    // Create the client socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        std::cerr << "Socket creation failed.\n";
    }

    // Set up the server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    // Convert IP address from string to binary
    if (inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address.\n";
    }
}

bool SocketClient::connectToServer() {
    std::cout << "Connecting to the server...\n";
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

}

void SocketClient::receiveData() {

}
