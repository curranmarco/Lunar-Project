#ifndef CSV_SENDER_HPP
#define CSV_SENDER_HPP

#include <string>

class CsvSender {
public:
    CsvSender(int socket_fd);
    bool sendCsvData(const std::string& csv_data);

private:
    int sock_fd;
};

#endif
