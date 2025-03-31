#include "packet_functions.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
Packet::Packet(uint16_t source, uint16_t dest)
    : source_port(source), dest_port(dest) {}

    #include <iostream>  // make sure this is at the top

    std::string Packet::SynPacket() {
        std::string src  = std::bitset<16>(source_port).to_string();
        std::string dst  = std::bitset<16>(dest_port).to_string();
        uint32_t seq_num = 10;  // generate as number so we can show both
        std::string seq  = std::bitset<32>(seq_num).to_string();
        std::string ack  = std::bitset<32>(0).to_string();
        std::string data_offset = std::bitset<4>(5).to_string(); // usually 5
        std::string reserved    = std::bitset<3>(0).to_string();
        std::string flags       = std::bitset<9>(0x02).to_string();  // SYN flag
        std::string offset_and_flags = data_offset + reserved + flags;
        std::string checksum_placeholder = std::bitset<16>(0).to_string();
    
        std::string header = src + dst + seq + ack + offset_and_flags + checksum_placeholder;
    
        uint16_t checksum = computeChecksum(header);
        std::string checksum_bits = std::bitset<16>(checksum).to_string();
    
        std::string syn_packet = src + dst + seq + ack + offset_and_flags + checksum_bits;
    
        // 👇 Debug printing
        std::cout << "[SYN Packet Breakdown]\n";
        std::cout << "Source Port     : " << src  << " (" << source_port << ")\n";
        std::cout << "Destination Port: " << dst  << " (" << dest_port  << ")\n";
        std::cout << "Sequence Number : " << seq  << " (" << seq_num    << ")\n";
        std::cout << "Acknowledgment  : " << ack  << " (0)\n";
        std::cout << "Offset+Flags    : " << offset_and_flags << " (0x" 
                  << std::hex << std::bitset<16>(offset_and_flags).to_ulong() << std::dec << ")\n";
        std::cout << "Checksum        : " << checksum_bits << " (" << checksum << ")\n";
        std::cout << "Full Packet     : " << syn_packet << "\n\n";
    
        return syn_packet;
    }
    
    

std::string Packet::SynAckPacket(uint32_t isnc) {
    std::string synack;
    std::string src = std::bitset<16>(source_port).to_string();
    std::string dst = std::bitset<16>(dest_port).to_string();
    std::string seq = std::bitset<32>(rand()).to_string();
    std::string ack = std::bitset<32>(0).to_string();
    std::string data_offset = std::bitset<4>(5).to_string(); // "0101"
    std::string reserved = std::bitset<3>(0).to_string(); 
    std::string flags = std::bitset<9>(0x12).to_string(); //syn ack flags 
    std::string offset_and_flags = data_offset + reserved + flags;
    std::string checksum_placeholder = std::bitset<16>(0).to_string();

    std::string headersynack = src + " " + dst + " " + seq + " " + ack + " " + offset_and_flags + " " + checksum_placeholder;

    uint16_t checksum = computeChecksum(headersynack);
    std::string checksum_bits = std::bitset<16>(checksum).to_string();

    synack= src + " " + dst + " " + seq + " " + ack + " " + offset_and_flags+ " " + checksum_bits;


    return synack; 
}

std::string Packet::AckPacket(uint32_t isns) {
    std::string src = std::bitset<16>(source_port).to_string();
    std::string dst = std::bitset<16>(dest_port).to_string();
    std::string seq = std::bitset<32>(rand()).to_string();
    std::string ack = std::bitset<32>(isns).to_string();
    std::string data_offset = std::bitset<4>(5).to_string();
    std::string reserved = std::bitset<3>(0).to_string(); 
    std::string flags = std::bitset<9>(0x10).to_string();  // ACK
    std::string offset_and_flags = data_offset + reserved + flags;
    std::string checksum_placeholder = std::bitset<16>(0).to_string();

    std::string headerack = src + dst  + seq  + ack + offset_and_flags + " " + checksum_placeholder;

    uint16_t checksum = computeChecksum(headerack);
    std::string checksum_bits = std::bitset<16>(checksum).to_string();

    std::string ack_packet = src + " " + dst + " " + seq + " " + ack + " " + offset_and_flags + " " + checksum_bits;
    return ack_packet;
}


std::string Packet::FinPacket() {
    std::string fin; 
    std::string src = std::bitset<16>(source_port).to_string();
    std::string dst = std::bitset<16>(dest_port).to_string();
    std::string seq = std::bitset<32>(rand()).to_string();
    std::string ack = std::bitset<32>(0).to_string();
    std::string data_offset = std::bitset<4>(5).to_string(); // "0101"
    std::string reserved = std::bitset<3>(0).to_string(); 
    std::string flags = std::bitset<9>(0x01).to_string(); //syn ack flags 
    std::string offset_and_flags = data_offset + reserved + flags;
    std::string checksum_placeholder = std::bitset<16>(0).to_string();

   std:: string headerfin = src + " " + dst + " " + seq + " " + ack + " " + offset_and_flags + " " + checksum_placeholder;

    uint16_t checksum = computeChecksum(headerfin);
    std::string checksum_bits = std::bitset<16>(checksum).to_string();

    fin = src + " " + dst + " " + seq + " " + ack + " " + offset_and_flags+ " " + checksum_bits;


    return fin; 
}


std::string Packet::DataPacket(uint32_t seq_num, uint32_t ack_num, const std::string& payload) {
    // Header fields
    std::string src = std::bitset<16>(source_port).to_string();
    std::string dst = std::bitset<16>(dest_port).to_string();
    std::string seq = std::bitset<32>(seq_num).to_string();
    std::string ack = std::bitset<32>(ack_num).to_string();

    // TCP-style offset/reserved/flags field (16 bits total)
    std::string data_offset = std::bitset<4>(5).to_string();        // header = 5 words
    std::string reserved = std::bitset<3>(0).to_string();           // unused
    std::string flags = std::bitset<9>(0x10).to_string();           // ACK
    std::string offset_and_flags = data_offset + reserved + flags; // 16 bits total

    std::string length = std::bitset<32>(payload.size()).to_string(); // payload length (bytes)
    std::string checksum_placeholder = std::bitset<16>(0).to_string();

    // Convert payload to 8-bit binary per character
    std::string payload_bits;
    for (char c : payload) {
        payload_bits += std::bitset<8>(static_cast<unsigned char>(c)).to_string();
    }

    // Pad payload to 32-bit boundary if needed
    size_t pad_bits = (32 - (payload_bits.size() % 32)) % 32;
    payload_bits += std::string(pad_bits, '0');

    // Optional: space every 8 bits for readability (can skip for pure binary stream)
    std::string spaced_payload;
    for (size_t i = 0; i < payload_bits.size(); i += 8) {
        spaced_payload += payload_bits.substr(i, 8) + " ";
    }
    if (!spaced_payload.empty()) {
        spaced_payload.pop_back(); // remove last space
    }

    // Header with placeholder checksum
    std::string header = src + " " + dst + " " + seq + " " + ack + " " + offset_and_flags + " " + length + " " + checksum_placeholder;
    std::string full_packet = header + " " + spaced_payload;

    // Compute checksum
    uint16_t checksum = computeChecksum(full_packet);
    std::string checksum_bits = std::bitset<16>(checksum).to_string();

    // Final packet with actual checksum
    std::string final_packet = src + " " + dst + " " + seq + " " + ack + " " + offset_and_flags + " " + length + " " + checksum_bits;
    if (!spaced_payload.empty()) {
        final_packet += " " + spaced_payload;
    }

    return final_packet;
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
    if (packet.length() < 16) return false;  // Must be at least one checksum field

    std::string data = packet.substr(0, packet.length() - 16);       // All bits before checksum
    std::string checksumBits = packet.substr(packet.length() - 16);  // Last 16 bits = checksum

    if (checksumBits.find_first_not_of("01") != std::string::npos)
        return false;

    uint16_t received = std::bitset<16>(checksumBits).to_ulong();
    uint16_t calculated = computeChecksum(data);

    std::cout << "[VerifyChecksum] Packet Length:  " << packet.length() << " bits\n";
    std::cout << "[VerifyChecksum] Data Bits:      " << data << "\n";
    std::cout << "[VerifyChecksum] Checksum Bits:  " << checksumBits << "\n";
    std::cout << "[VerifyChecksum] Received (dec): " << received << "\n";
    std::cout << "[VerifyChecksum] Calculated:     " << calculated << "\n";
    std::cout << "[VerifyChecksum] Match?          " << (received == calculated ? "✅ YES" : "❌ NO") << "\n";

    return received == calculated;
}
