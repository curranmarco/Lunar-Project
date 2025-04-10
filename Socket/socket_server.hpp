#ifndef SOCKETSERVER_H
#define SOCKETSERVER_H

#include <iostream> // For standard input/output
#include <sys/socket.h> // For socket-related functions
#include <netinet/in.h> // For sockaddr_in structure
#include <unistd.h> // For close() function
#include <bitset> // For bit manipulation
#include <netinet/in.h> // For network-related constants
#include "../Packet/packet_functions.hpp" // Include packet-related functions

// Forward declaration of the Packet class
class Packet;

// SocketServer class definition
class SocketServer {
    friend class Packet; // Allow the Packet class to access private members of SocketServer

private:
    int server_fd, client_socket; // File descriptors for the server and client sockets
    struct sockaddr_in local_addr; // Structure to store the server's address
    struct sockaddr_in client_addr; // Structure to store the client's address
    int addrlen; // Length of the address structure
    u_int32_t isn; // Initial sequence number for the connection

public:
    // Constructor to initialize the server with a specific port
    explicit SocketServer(int port);

    // Function to start listening for incoming connections
    bool startListening();

    // Function to accept a client connection
    bool acceptClient();

    // Function to close the current client connection
    void closeConnection();

    // Destructor to clean up resources
    ~SocketServer();

    // Function to get the client socket file descriptor
    int getClientSocket() const;
};

#endif

/* ALL PACKET RELATED FUNCTIONS HAVE BEEN MOVED TO PACKET DIRECTORY */