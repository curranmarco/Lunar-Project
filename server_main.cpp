#include "Socket/socket_server.hpp"
#include "Packet/packet_functions.hpp"

#include <iostream>
#include <bitset>
#include <string>
#include <cstring>  // for memset
#include <unistd.h> // for read()

void processPacket(const std::string& packet) {
    std::cout << "\n[Raw Packet Received]\n" << packet << "\n";

    if (!Packet::verifyChecksum(packet)) {
        std::cerr << "❌ Invalid checksum. Packet corrupted.\n";
        return;
    }

    std::cout << "✅ Checksum valid.\n";

    // Split packet to extract payload
    size_t firstPayloadBit = 16 + 1 + 16 + 1 + 32 + 1 + 32 + 1 + 32 + 1 + 32 + 1; // Up to payload
    std::string payload_and_checksum = packet.substr(firstPayloadBit);
    size_t last_space = payload_and_checksum.rfind(' ');
    if (last_space == std::string::npos) {
        std::cerr << "❌ Payload extraction failed.\n";
        return;
    }

    std::string payload = payload_and_checksum.substr(0, last_space);

    if (payload.length() < 20) {
        std::cerr << "❌ Payload too short to extract data.\n";
        return;
    }

    std::string location_bits = payload.substr(0, 4);
    std::string moisture_bits = payload.substr(4, 16);

    int location_id = std::bitset<4>(location_bits).to_ulong();
    int moisture_value = std::bitset<16>(moisture_bits).to_ulong();

    std::cout << "\n[Sensor Data Extracted]\n";
    std::cout << "Location ID:     " << location_id << "\n";
    std::cout << "Moisture Value:  " << moisture_value << "\n";
}

int main() {
    int port = 8080; // Port to listen on
    SocketServer server(port);

    if (!server.startListening()) {
        return 1;
    }

    if (!server.acceptClient()) {
        return 1;
    }

    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    ssize_t bytes_received = read(server.getClientSocket(), buffer, sizeof(buffer));
    if (bytes_received <= 0) {
        std::cerr << "❌ No data received.\n";
        return 1;
    }

    std::string received_packet(buffer, bytes_received);
    processPacket(received_packet);

    return 0;
}


