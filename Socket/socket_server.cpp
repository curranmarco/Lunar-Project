#include "socket_server.hpp"

SocketServer::SocketServer(int port) {
    addrlen = sizeof(address);

    // Create the server socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Socket creation failed.\n";
    }

    // Set up the server address structure
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Bind the socket
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
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
    client_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
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
