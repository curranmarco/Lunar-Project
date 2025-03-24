#include <winsock2.h>
#include <iostream>
#include <vector>
#include <string>

using namespace std;

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    SOCKET clientSocket;
    sockaddr_in serverAddr;
    char buffer[1024];

    // Initialize Winsock
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // Create socket
    clientSocket = socket(AF_INET, SOCK_DGRAM, 0);

    // Server address setup
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(54000);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Change to server IP if needed

    // Step 1: Send "start" to the server
    string startMsg = "start";
    sendto(clientSocket, startMsg.c_str(), startMsg.size(), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
    cout << "Sent start request to server.\n";

    // Step 2: Receive total count
    int serverAddrLen = sizeof(serverAddr);
    int bytesReceived = recvfrom(clientSocket, buffer, sizeof(buffer), 0, (sockaddr*)&serverAddr, &serverAddrLen);
    buffer[bytesReceived] = '\0';
    int totalCount = stoi(buffer);
    cout << "Expecting " << totalCount << " float values from server.\n";

    // Step 3: Receive float values
    vector<float> receivedValues;
    for (int i = 0; i < totalCount; ++i) {
        bytesReceived = recvfrom(clientSocket, buffer, sizeof(buffer), 0, (sockaddr*)&serverAddr, &serverAddrLen);
        buffer[bytesReceived] = '\0';
        try {
            float val = stof(buffer);
            receivedValues.push_back(val);
            cout << "Received: " << val << "\n";
        } catch (...) {
            cerr << "Invalid float received: " << buffer << "\n";
        }
    }

    cout << "All values received.\n";

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}



