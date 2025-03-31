#include "Packet/packet_functions.hpp"
#include <iostream>

int main() {
    // Create a Packet object with source and destination ports
    Packet packet(1234, 8080);

    // --- Step 1: Create a SYN packet
    std::string syn_packet = packet.SynPacket();

    // --- Step 2: Print the full SYN packet
    std::cout << "\n=== [Test] SYN Packet Created ===\n";
    std::cout << syn_packet << "\n";
    std::cout << "Length: " << syn_packet.length() << " bits\n";

    // --- Step 3: Verify the checksum
    std::cout << "\n=== [Test] Verifying Checksum ===\n";
    bool valid = packet.verifyChecksum(syn_packet);

    std::cout << "\nResult: " << (valid ? " Packet is valid" : "Packet is invalid") << "\n";

    return 0;
}