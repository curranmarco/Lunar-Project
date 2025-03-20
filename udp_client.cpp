#include <iostream>
#include <sys/socket.h> // Socket programming
#include <netinet/in.h> // IP Address structures
#include <arpa/inet.h> 
#include <unistd.h>

#define SERVER_IP "127.0.0.1"  // Use actual IP if running on different machines
#define SERVER_PORT 5000

int main() {
    // Create a UDP socket
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return -1;
    }

    // Server address structure
    struct sockaddr_in serverAddr;
    // Set server address
    serverAddr.sin_family = AF_INET;
    // Set ports number
    serverAddr.sin_port = htons(SERVER_PORT);
    // Convert IP address to binary form
    inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

    // Send a message to the server
    std::cout << "Connecting to Ground Station...\n";
    char dummyMessage[1] = {0};  // Empty message to simulate connection
    // Send message to server
    sendto(sock, dummyMessage, sizeof(dummyMessage), 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

    std::cout << "Connection successful!\n";

    close(sock);
    return 0;
}
