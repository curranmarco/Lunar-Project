#ifndef PACKET_HPP
#define PACKET_HPP

// Enum representing different packet types in the custom protocol
enum class PacketType {
    SYN,
    ACK,
    DATA,
    FIN
};

// Optional helper function to convert enum to string (for logging)
inline const char* to_string(PacketType type) {
    switch (type) {
        case PacketType::SYN:  return "SYN";
        case PacketType::ACK:  return "ACK";
        case PacketType::DATA: return "DATA";
        case PacketType::FIN:  return "FIN";
        default:               return "UNKNOWN";
    }
}

#endif // PACKET_HPP

