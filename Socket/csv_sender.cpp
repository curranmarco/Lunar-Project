#include "csv_sender.hpp"
#include <iostream>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#endif
CsvSender::CsvSender(int socket_fd) : sock_fd(socket_fd) {}

bool CsvSender::sendCsvData(const std::string& csv_data) {
    std::string data = csv_data + "\n"; // add newline as delimiter
    if (send(sock_fd, data.c_str(), data.size(), 0) < 0) {
        std::cerr << "Failed to send CSV data.\n";
        return false;
    }
    return true;
}
