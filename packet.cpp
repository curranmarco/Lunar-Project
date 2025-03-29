#include "packet.hpp"

// Encode a Packet into raw binary bytes
std::vector<uint8_t> encodePacket(const Packet& packet) {
    std::vector<uint8_t> data;
    // First byte is the type, second byte is the ID
    data.push_back(static_cast<uint8_t>(packet.type));
    data.push_back(packet.id);

    // Next two bytes are the size of the payload
    uint16_t size = packet.payload.size();
    data.push_back(size >> 8);        // High byte
    data.push_back(size & 0xFF);      // Low byte

    // Append the payload to the data
    data.insert(data.end(), packet.payload.begin(), packet.payload.end());
    return data;
}

// Decode raw binary bytes into a Packet
Packet decodePacket(const std::vector<uint8_t>& raw) {
    // Create an empty packet
    Packet packet;
    // Check if there's space for the header (type, id, size)
    if (raw.size() < 4) return packet;

    // Extract the type, id, and size from the raw data
    packet.type = static_cast<PacketType>(raw[0]);
    packet.id = raw[1];
    // Extract the size of the payload
    uint16_t size = (raw[2] << 8) | raw[3];

    // Extract the payload if it fits in the raw data
    if (raw.size() >= 4 + size) {
        packet.payload.assign(raw.begin() + 4, raw.begin() + 4 + size);
    }
    return packet;
}
