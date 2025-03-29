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