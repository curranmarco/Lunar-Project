#ifndef PACKET_HPP
#define PACKET_HPP

#include <string> // For handling strings
#include <bitset> // For bit manipulation
#include <cstdint> // For fixed-width integer types

// The Packet class provides functionality for creating, sending, and verifying network packets
class Packet {
public:
    // Constructor: Initializes the packet with source and destination ports
    Packet(uint16_t source_port, uint16_t dest_port);

    // Function to create a SYN packet (used to initiate a connection)
    std::string SynPacket();

    // Function to create a SYN-ACK packet (used to acknowledge a SYN packet)
    std::string SynAckPacket(uint32_t isnc);

    // Function to create an ACK packet (used to acknowledge received data or control packets)
    std::string AckPacket(uint32_t isns);

    // Function to create a FIN packet (used to terminate a connection)
    std::string FinPacket();

    // Function to create a data packet with a sequence number, acknowledgment number, payload, and flags
    std::string DataPacket(uint32_t seq_num, uint32_t ack_num, const std::string& payload, uint16_t flag);

    // Static function to compute the checksum of a given data string
    static uint16_t computeChecksum(const std::string& data);

    // Function to send a packet over a socket
    bool SendPacket(int socket_fd, const std::string& packet);

    // Static function to verify the checksum of a received packet
    static bool verifyChecksum(const std::string& packet);

private:
    uint16_t source_port; // The source port of the packet
    uint16_t dest_port; // The destination port of the packet
};

#endif // PACKET_HPP