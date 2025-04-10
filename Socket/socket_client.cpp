#include "socket_client.hpp" // Include the header file for the SocketClient class

// Constructor: Initializes the client socket and sets up the server address
SocketClient::SocketClient(const std::string& server_ip, int port) {
    // Create the client socket
    sock = socket(AF_INET, SOCK_STREAM, 0); // Use IPv4 and TCP
    if (sock == -1) {
        std::cerr << "Socket creation failed.\n"; // Print an error message if socket creation fails
    }

    // Set up the server address structure
    server_addr.sin_family = AF_INET; // Use IPv4
    server_addr.sin_port = htons(port); // Convert the port number to network byte order

    // Convert the server IP address from string to binary format
    if (inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address.\n"; // Print an error message if the IP address is invalid
    }
}

// Function to connect to the server, optionally binding to a specific source port
bool SocketClient::connectToServer(int source_port) {
    std::cout << "Connecting to the server...\n";

    // If a source port is specified, bind the client socket to it
    if (source_port > 0) {
        local_addr.sin_family = AF_INET; // Use IPv4
        local_addr.sin_addr.s_addr = INADDR_ANY; // Bind to any available local interface
        local_addr.sin_port = htons(source_port); // Convert the source port to network byte order

        // Bind the client socket to the specified source port
        if (bind(sock, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
            std::cerr << "Failed to bind source port " << source_port << ".\n"; // Print an error message if binding fails
            return false;
        }
        std::cout << "Bound to source port: " << source_port << "\n"; // Indicate successful binding
    }

    // Attempt to connect to the server
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connection failed.\n"; // Print an error message if the connection fails
        return false;
    }
    std::cout << "Connected to the server.\n"; // Indicate successful connection
    return true;
}

// Function to close the connection to the server
void SocketClient::closeConnection() {
    close(sock); // Close the client socket
}

// Destructor: Ensures that the connection is closed when the object is destroyed
SocketClient::~SocketClient() {
    closeConnection(); // Close the connection
}

// Function to get the client socket file descriptor
int SocketClient::getSocket() const {
    return sock; // Return the file descriptor for the client socket
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

/* ALL PACKET RELATED FUNCTIONS HAVE BEEN MOVED TO PACKET DIRECTORY */