#ifndef PACKET_FUNCTIONS_HPP
#define PACKET_FUNCTIONS_HPP

#include <string>
#include <bitset>
#include <cstdint>

class Packet {
private:
    uint16_t source_port;
    uint16_t dest_port;

public:
    Packet(uint16_t source, uint16_t dest);

    std::string SynPacket();
    std::string SynAckPacket(uint32_t isnc);
    std::string AckPacket(uint32_t isns);
    std::string FinPacket();
    std::string DataPacket(uint32_t seq_num, uint32_t ack_num, const std::string& payload, uint16_t flags = 0x10);

    static bool SendPacket(int socket_fd, const std::string& packet);
    static uint16_t computeChecksum(const std::string& data);
    static bool verifyChecksum(const std::string& packet);

private:
    std::string buildHeaderField(uint16_t flags);
};

#endif // PACKET_FUNCTIONS_HPP
