#include <winsock2.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <thread> // for sleep
#include <chrono>

using namespace std;

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    SOCKET serverSocket;
    sockaddr_in serverAddr, clientAddr;
    char buffer[1024];

    WSAStartup(MAKEWORD(2,2), &wsaData);
    serverSocket = socket(AF_INET, SOCK_DGRAM, 0);

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(54000);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    // will associate the socket with a specific local IP address and Port. 
    //serverAdder tells the application to listen on a specific port or IP
    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    cout << "Server running... waiting for client to start.\n";
    //will store the client's IP and port info 
    int clientLen = sizeof(clientAddr);
    //line used that waits for a udp packet to arrive 
    int bytesReceived = recvfrom(serverSocket, buffer, sizeof(buffer), 0, (sockaddr*)&clientAddr, &clientLen);
    buffer[bytesReceived] = '\0';

    if (string(buffer) == "start") {
        cout << "Client initiated stream.\n";

        // Step 1: Read CSV and extract all float values
        ifstream file("lunarwater.csv");
        if (!file.is_open()) {
            cerr << "Failed to open lunarwater.csv\n";
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }

        vector<float> values;
        string line;

        while (getline(file, line)) {
            stringstream ss(line); //working on a string in memory 
            string item;
            // while loop that will assign the parsed data into the item string 
            //will convert them into a float and skip any potential harmful values 
            while (getline(ss, item, ',')) {
                try {
                    values.push_back(stof(item));
                } catch (...) {
                    cerr << "Skipping invalid value: " << item << "\n";
                }
            }
        }

        file.close();

        cout << "Total float values to send: " << values.size() << "\n";

        // Step 2: Send the total count as the first packet
        string countStr = to_string(values.size());
        sendto(serverSocket, countStr.c_str(), countStr.size(), 0, (sockaddr*)&clientAddr, clientLen);

        // Step 3: Send each float value in a separate packet
        for (float val : values) {
            string valueStr = to_string(val);
            sendto(serverSocket, valueStr.c_str(), valueStr.size(), 0, (sockaddr*)&clientAddr, clientLen);
            cout << "Sent value: " << valueStr << "\n";
            this_thread::sleep_for(chrono::milliseconds(100)); // Optional: prevent flooding
        }

        cout << "All values sent.\n";
    } else {
        cout << "Unexpected client message: " << buffer << "\n";
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}





