#include "csv_receiver.hpp"
#include <iostream>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#endif
CsvReceiver::CsvReceiver(int socket_fd) : sock_fd(socket_fd) {}

std::string CsvReceiver::receiveCsvData() {
    char ch;
    std::string data;

    // Read until newline (delimiter)
    while (recv(sock_fd, &ch, 1, 0) > 0 && ch != '\n') {
        data += ch;
    }

    return data;
}

