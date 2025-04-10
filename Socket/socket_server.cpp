#include "socket_server.hpp" // Include the header file for the SocketServer class

// Constructor: Initializes the server socket and binds it to the specified port
SocketServer::SocketServer(int port) {
    addrlen = sizeof(local_addr); // Set the length of the address structure

    // Create the server socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0); // Use IPv4 and TCP
    if (server_fd == -1) {
        std::cerr << "Socket creation failed.\n"; // Print an error message if socket creation fails
    }

    // Set up the server address structure
    local_addr.sin_family = AF_INET; // Use IPv4
    local_addr.sin_addr.s_addr = INADDR_ANY; // Accept connections from any IP address
    local_addr.sin_port = htons(port); // Convert the port number to network byte order

    // Bind the socket to the specified port
    if (bind(server_fd, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
        std::cerr << "Bind failed.\n"; // Print an error message if binding fails
    }
}

// Function to start listening for incoming connections
bool SocketServer::startListening() {
    // Start listening on the server socket with a backlog of 1 connection
    if (listen(server_fd, 1) < 0) {
        std::cerr << "Listen failed.\n"; // Print an error message if listening fails
        return false;
    }
    std::cout << "Waiting for a connection...\n"; // Indicate that the server is ready to accept connections
    return true;
}

// Function to accept a client connection
bool SocketServer::acceptClient() {
    // Accept an incoming connection and store the client's socket
    client_socket = accept(server_fd, (struct sockaddr*)&client_addr, (socklen_t*)&addrlen);
    if (client_socket < 0) {
        std::cerr << "Accept failed.\n"; // Print an error message if accepting the connection fails
        return false;
    }
    std::cout << "Connection established.\n"; // Indicate that a connection has been successfully established
    return true;
}

// Function to close the current client connection and the server socket
void SocketServer::closeConnection() {
    close(client_socket); // Close the client socket
    close(server_fd); // Close the server socket
}

// Destructor: Ensures that all resources are cleaned up when the object is destroyed
SocketServer::~SocketServer() {
    closeConnection(); // Close any open connections
}

// Function to get the client socket file descriptor
int SocketServer::getClientSocket() const {
    return client_socket; // Return the file descriptor for the client socket
}

/* ALL PACKET RELATED FUNCTIONS HAVE BEEN MOVED TO PACKET DIRECTORY */