#include "../socket_client.hpp"


#include "../csv_sender.hpp"
#include <iostream>

int main() {
    SocketClient client("127.0.0.1", 8080);
    if (!client.connectToServer()) return 1;

    CsvSender sender(client.getSocket());
    std::string csv;
    std::cout << "Enter CSV values to send (e.g., 1,2,3): ";
    std::getline(std::cin, csv);
    sender.sendCsvData(csv);

    client.closeConnection();
    return 0;
}
