#ifndef CSV_RECEIVER_HPP
#define CSV_RECEIVER_HPP

#include <string>

class CsvReceiver {
public:
    CsvReceiver(int socket_fd);
    std::string receiveCsvData();

private:
    int sock_fd;
};

#endif
