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
    static uint16_t computeChecksum(const std::string& data);
    bool SendPacket(int socket_fd, const std::string& packet);
    static bool verifyChecksum(const std::string& packet);
    void parsePacket(const std::string& packet); 
    uint32_t last_seq_num = 0;
    uint32_t last_ack_num = 0;
private:
    
    uint32_t initial_seq_num = 0;
    bool isn_initialized = false;

 
    uint16_t source_port;
    uint16_t dest_port;
};

#endif // PACKET_HPP
