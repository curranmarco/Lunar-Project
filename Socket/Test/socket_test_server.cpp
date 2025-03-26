#include "../socket_server.hpp"

#include "../csv_receiver.hpp"
#include <iostream>

int main() {
    SocketServer server(8080);
    if (!server.startListening()) return 1;
    if (!server.acceptClient()) return 1;

    CsvReceiver receiver(server.getClientSocket());
    std::string csv = receiver.receiveCsvData();
    std::cout << "Received CSV values: " << csv << "\n";

    server.closeConnection();
    return 0;
}