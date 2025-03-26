#include "socket_client.hpp"
#include "csv_sender.hpp"
#include <iostream>

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#endif

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    int wsaInit = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaInit != 0) {
        std::cerr << "WSAStartup failed: " << wsaInit << "\n";
        return 1;
    }
#endif

    SocketClient client("127.0.0.1", 8080);
    if (!client.connectToServer()) return 1;

    CsvSender sender(client.getSocket());
    std::string csv;
    std::cout << "Enter CSV values to send (e.g., 1,2,3): ";
    std::getline(std::cin, csv);
    sender.sendCsvData(csv);

    client.closeConnection();

#ifdef _WIN32
    WSACleanup();  // Clean up Winsock
#endif

    return 0;
}
