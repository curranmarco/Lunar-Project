#include <winsock2.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <ctime>

#pragma comment(lib, "ws2_32.lib")

std::string generateCSV() {
    // Generate a random CSV string
    float a = rand() % 100 + (rand() % 100) / 100.0f;
    float b = rand() % 100 + (rand() % 100) / 100.0f;
    float c = rand() % 100 + (rand() % 100) / 100.0f;
    float d = rand() % 100 + (rand() % 100) / 100.0f;

    return std::to_string(a) + "," + std::to_string(b) + "," +
           std::to_string(c) + "," + std::to_string(d);
}

int main() {
    WSADATA wsaData;
    SOCKET serverSocket;
    sockaddr_in serverAddr, clientAddr;
    int clientLen = sizeof(clientAddr);
    char buffer[1024];

    srand((unsigned)time(0));

    // Initialize Winsock
    WSAStartup(MAKEWORD(2,2), &wsaData);

    // Create UDP socket
    serverSocket = socket(AF_INET, SOCK_DGRAM, 0);

    // Bind socket
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(54000);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));

    std::cout << "Waiting for client...\n";

    // Wait for a client to contact us first
    recvfrom(serverSocket, buffer, sizeof(buffer), 0, (sockaddr*)&clientAddr, &clientLen);
    std::cout << "Client connected, starting stream...\n";

    // Stream CSV packets every second
    for (int i = 0; i < 20; ++i) { // or while(true)
        std::string csv = generateCSV();
        sendto(serverSocket, csv.c_str(), csv.size(), 0, (sockaddr*)&clientAddr, clientLen);
        std::cout << "Sent: " << csv << "\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}

