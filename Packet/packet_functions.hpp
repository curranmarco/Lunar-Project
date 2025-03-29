#ifndef PACKET_HPP
#define PACKET_HPP

#include <string>
#include <bitset>
#include <cstdint>

class Packet {
public:
    Packet(uint16_t source_port, uint16_t dest_port);

    std::string SynPacket();
    std::string SynAckPacket(uint32_t isnc); 
    std::string AckPacket(uint32_t isns);
    std::string FinPacket();
    std::string DataPacket(uint32_t seq_num, uint32_t ack_num, const std::string& payload);

    bool SendPacket(int socket_fd, const std::string& packet);
    bool verifyChecksum(const std::string& packet);

private:
    uint16_t computeChecksum(const std::string& data);

    uint16_t source_port;
    uint16_t dest_port;
};

#endif // PACKET_HPP
