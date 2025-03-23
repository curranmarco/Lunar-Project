#include <winsock2.h>
#include <iostream>
#include <sstream>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    SOCKET clientSocket;
    sockaddr_in serverAddr;
    char buffer[1024];

    WSAStartup(MAKEWORD(2,2), &wsaData);
    clientSocket = socket(AF_INET, SOCK_DGRAM, 0);

    // Server info
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(54000);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Send a dummy packet to initiate stream
    std::string hello = "start";
    sendto(clientSocket, hello.c_str(), hello.size(), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));

    // Receive streamed CSV packets
    for (int i = 0; i < 20; ++i) {
        int serverLen = sizeof(serverAddr);
        int bytesReceived = recvfrom(clientSocket, buffer, sizeof(buffer), 0, (sockaddr*)&serverAddr, &serverLen);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            std::cout << "Received CSV: " << buffer << "\n";

            // Parse CSV
            std::stringstream ss(buffer);
            std::string item;
            std::vector<float> values;
            while (std::getline(ss, item, ',')) {
                values.push_back(std::stof(item));
            }

            std::cout << "Parsed Values: ";
            for (float v : values) std::cout << v << " ";
            std::cout << "\n\n";
        }
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
