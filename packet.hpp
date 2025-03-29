#pragma once
#include <vector>
#include <cstdint>

// Packet type
enum PacketType : uint8_t {
    SENSOR = 0x00,
    ACTUATOR = 0x01
};

// Packet structure
struct Packet {
    PacketType type;
    uint8_t id;
    std::vector<uint8_t> payload;
};

// Encode a Packet into raw binary bytes
std::vector<uint8_t> encodePacket(const Packet& packet);

// Decode raw binary bytes into a Packet
Packet decodePacket(const std::vector<uint8_t>& raw);
