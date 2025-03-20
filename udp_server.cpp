#include <iostream>
#include <sys/socket.h> // Socket programming
#include <netinet/in.h> // IP Address structures
#include <unistd.h> // Closing sockets

// Port server is listening on
#define SERVER_PORT 5000

int main() {
    // UDP Socket creation. IPv4, Datagram (UDP), Protocol 0
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return -1;
    }

    // Server address structure
    struct sockaddr_in serverAddr, clientAddr;
    serverAddr.sin_family = AF_INET;
    // Allows server to listen on any IP address
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    // Convert port to network byte order
    serverAddr.sin_port = htons(SERVER_PORT);

    // Bind socket to server address
    if (bind(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind failed");
        close(sock);
        return -1;
    }

    std::cout << "UDP Server (Ground Station) is waiting for connection...\n";

    // Receive data from client
    char buffer[1];  // Dummy buffer, we are not expecting data
    // clientAddr will be populated with client's address
    socklen_t len = sizeof(clientAddr);
    // Waits for a message from client
    recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&clientAddr, &len);

    std::cout << "Rover connected!\n";

    close(sock);
    return 0;
}
