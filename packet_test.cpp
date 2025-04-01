#include "Packet/packet_functions.hpp"
#include <iostream>
#include <string>

int main() {
    Packet packet(1234, 8080);  // arbitrary ports for testing

    // Test SYN
    std::string syn = packet.SynPacket();
    std::cout << ">>> Testing SYN Packet\n";
    packet.verifyChecksum(syn);
    packet.parsePacket(syn);

    // Test SYN-ACK
    std::string synack = packet.SynAckPacket(11);  // assume client seq + 1 = 11
    std::cout << "\n>>> Testing SYN-ACK Packet\n";
    packet.verifyChecksum(synack);
    packet.parsePacket(synack);

    // Test ACK
    std::string ack = packet.AckPacket(99);  // assume server seq + 1 = 99
    std::cout << "\n>>> Testing ACK Packet\n";
    packet.verifyChecksum(ack);
    packet.parsePacket(ack);

    // Test FIN
    std::string fin = packet.FinPacket();
    std::cout << "\n>>> Testing FIN Packet\n";
    packet.verifyChecksum(fin);
    packet.parsePacket(fin);

    // Test DATA
    std::string fake_payload = "0101010101101100";  // 16-bit dummy data
    std::string data = packet.DataPacket(1, 1, fake_payload);
    std::cout << "\n>>> Testing DATA Packet\n";
    packet.verifyChecksum(data);
    packet.parsePacket(data);

    return 0;
}
