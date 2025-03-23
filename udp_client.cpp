#include <iostream>
#include <sys/socket.h> // Socket programming
#include <netinet/in.h> // IP Address structures
#include <arpa/inet.h> 
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <ctime>

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
    srand(time(0));
    // Keep sending data until stopped manually
    while (true){
        // Generate random temperature and pressure data
        char data[50];
        int temp = rand() % 50;
        int pressure = rand() % 1000 +900;
        
        // Format data
        snprintf(data, sizeof(data), "Temperature: %d, Pressure: %d", temp, pressure);
        // Send data to server
        sendto(sock, data, strlen(data), 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
        std::cout << "Data sent to Ground Station: " << data << std::endl;
        sleep(2);
    }
    

    close(sock);
    return 0;
}
