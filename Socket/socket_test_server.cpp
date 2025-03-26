#include "socket_server.hpp"
#include "csv_receiver.hpp"
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

    SocketServer server(8080);
    if (!server.startListening()) return 1;
    if (!server.acceptClient()) return 1;

    CsvReceiver receiver(server.getClientSocket());
    std::string csv = receiver.receiveCsvData();
    std::cout << "Received CSV values: " << csv << "\n";

    server.closeConnection();

#ifdef _WIN32
    WSACleanup();  // Clean up Winsock
#endif

    return 0;
}
