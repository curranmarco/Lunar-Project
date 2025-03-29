#include "packet_functions.hpp"
#include <sys/socket.h>
#include <unistd.h>

Packet::Packet(uint16_t source, uint16_t dest)
    : source_port(source), dest_port(dest) {}

std::string Packet::SynPacket() {
    std::string syn;
    syn += std::bitset<16>(source_port).to_string() + " ";
    syn += std::bitset<16>(dest_port).to_string() + " ";
    syn += std::bitset<32>(rand()).to_string() + " ";
    syn += std::bitset<32>(0).to_string() + " ";
    syn += std::bitset<32>(0x02).to_string(); // SYN
    uint16_t checksum = computeChecksum(syn);
    return syn + " " + std::bitset<16>(checksum).to_string();
}

std::string Packet::SynAckPacket(uint32_t isnc) {
    std::string packet;
    packet += std::bitset<16>(source_port).to_string() + " ";
    packet += std::bitset<16>(dest_port).to_string() + " ";
    packet += std::bitset<32>(isnc).to_string() + " ";
    packet += std::bitset<32>(isnc + 1).to_string() + " ";
    packet += std::bitset<32>(0x12).to_string(); // SYN + ACK
    uint16_t checksum = computeChecksum(packet);
    return packet + " " + std::bitset<16>(checksum).to_string();
}

std::string Packet::AckPacket(uint32_t isns) {
    std::string ack;
    ack += std::bitset<16>(source_port).to_string() + " ";
    ack += std::bitset<16>(dest_port).to_string() + " ";
    ack += std::bitset<32>(isns).to_string() + " ";
    ack += std::bitset<32>(isns + 1).to_string() + " ";
    ack += std::bitset<32>(0x10).to_string(); // ACK
    uint16_t checksum = computeChecksum(ack);
    return ack + " " + std::bitset<16>(checksum).to_string();
}

std::string Packet::FinPacket() {
    std::string fin;
    fin += std::bitset<16>(source_port).to_string() + " ";
    fin += std::bitset<16>(dest_port).to_string() + " ";
    fin += std::bitset<32>(rand()).to_string() + " ";
    fin += std::bitset<32>(0).to_string() + " ";
    fin += std::bitset<32>(0x01).to_string(); // FIN
    uint16_t checksum = computeChecksum(fin);
    return fin + " " + std::bitset<16>(checksum).to_string();
}

std::string Packet::DataPacket(uint32_t seq_num, uint32_t ack_num, const std::string& payload) {
    std::string data;
    data += std::bitset<16>(source_port).to_string() + " ";
    data += std::bitset<16>(dest_port).to_string() + " ";
    data += std::bitset<32>(seq_num).to_string() + " ";
    data += std::bitset<32>(ack_num).to_string() + " ";
    data += std::bitset<32>(0x10).to_string() + " "; // ACK
    data += std::bitset<32>(payload.size()).to_string() + " ";
    data += payload;

    uint16_t checksum = computeChecksum(data);
    return data + " " + std::bitset<16>(checksum).to_string();
}

bool Packet::SendPacket(int socket_fd, const std::string& packet) {
    ssize_t sent = send(socket_fd, packet.c_str(), packet.size(), 0);
    return sent == (ssize_t)packet.size();
}

uint16_t Packet::computeChecksum(const std::string& data) {
    uint32_t sum = 0;
    for (size_t i = 0; i < data.size(); i += 2) {
        uint16_t part = static_cast<unsigned char>(data[i]) << 8;
        if (i + 1 < data.size()) {
            part |= static_cast<unsigned char>(data[i + 1]);
        }
        sum += part;
        if (sum > 0xFFFF) {
            sum = (sum & 0xFFFF) + 1;  // wrap around
        }
    }
    return ~sum & 0xFFFF;
}

bool Packet::verifyChecksum(const std::string& packet) {
    size_t lastSpace = packet.rfind(' ');
    if (lastSpace == std::string::npos) return false;

    std::string data = packet.substr(0, lastSpace);
    std::string checksumBits = packet.substr(lastSpace + 1);

    if (checksumBits.length() != 16 || checksumBits.find_first_not_of("01") != std::string::npos)
        return false;

    uint16_t received = std::bitset<16>(checksumBits).to_ulong();
    uint16_t calculated = computeChecksum(data);

    return received == calculated;
}
