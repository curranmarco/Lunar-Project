#include "packet_functions.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
Packet::Packet(uint16_t source, uint16_t dest)
    : source_port(source), dest_port(dest) {}
    
   


    std::string Packet::SynPacket() {
        if (!isn_initialized) {
            initial_seq_num = rand();  // generate once
            isn_initialized = true;
        }
        std::string src  = std::bitset<16>(source_port).to_string();
        std::string dst  = std::bitset<16>(dest_port).to_string();
        uint32_t seq_num = 10;  // generate as number so we can show both
        std::string seq  = std::bitset<32>(seq_num).to_string();
        std::string ack  = std::bitset<32>(0).to_string();
        std::string data_offset = std::bitset<4>(5).to_string(); // usually 5
        std::string reserved    = std::bitset<3>(0).to_string();
        std::string flags       = std::bitset<9>(0x02).to_string();  // SYN flag
        std::string offset_and_flags = data_offset + reserved + flags;
        std::string urgent = std::bitset<16>(0).to_string();
        std::string window_size = std::bitset<16>(0x24).to_string();
        std::string checksum_placeholder = std::bitset<16>(0).to_string();
    
        std::string header = src + dst + seq + ack + offset_and_flags + window_size + checksum_placeholder + urgent;

        uint16_t checksum = computeChecksum(header);
        std::string checksum_bits = std::bitset<16>(checksum).to_string();
    
        std::string syn_packet = src + dst + seq + ack + offset_and_flags + window_size + checksum_bits + urgent;

    

    
        return syn_packet;
    }
    
    

    std::string Packet::SynAckPacket(uint32_t isnc) {

        if (!isn_initialized) {
            initial_seq_num = rand();  // Generate server's ISN once
            isn_initialized = true;
        }
        std::string src  = std::bitset<16>(source_port).to_string();
        std::string dst  = std::bitset<16>(dest_port).to_string();
        std::string seq  = std::bitset<32>(initial_seq_num).to_string();
        std::string ack  = std::bitset<32>(isnc).to_string();  // use client seq + 1 ideally
        std::string data_offset = std::bitset<4>(5).to_string();
        std::string reserved    = std::bitset<3>(0).to_string();
        std::string flags       = std::bitset<9>(0x12).to_string(); // SYN + ACK
        std::string offset_and_flags = data_offset + reserved + flags;
        std::string urgent = std::bitset<16>(0).to_string();
        std::string window_size = std::bitset<16>(0x24).to_string();
        std::string checksum_placeholder = std::bitset<16>(0).to_string();
    
        std::string header = src + dst + seq + ack + offset_and_flags + window_size + checksum_placeholder + urgent;

       
        uint16_t checksum = computeChecksum(header);
        std::string checksum_bits = std::bitset<16>(checksum).to_string();
    
        return src + dst + seq + ack + offset_and_flags + window_size + checksum_bits + urgent;

    }
    
    std::string Packet::AckPacket(uint32_t isns) {
        std::string src  = std::bitset<16>(source_port).to_string();
        std::string dst  = std::bitset<16>(dest_port).to_string();



        std::string seq  = std::bitset<32>(last_seq_num).to_string();

    // Acknowledge the sequence number we received from peer + 1 (for SYN/FIN) or data length
        uint32_t ack_num = isns + 1;  // You can make this smarter if you know payload length
        std::string ack  = std::bitset<32>(ack_num).to_string();;
    
        std::string data_offset = std::bitset<4>(5).to_string();
        std::string reserved    = std::bitset<3>(0).to_string();
        std::string flags       = std::bitset<9>(0x10).to_string(); // ACK only
        std::string offset_and_flags = data_offset + reserved + flags;
    
        std::string window_size = std::bitset<16>(0x24).to_string();
        std::string checksum_placeholder = std::bitset<16>(0).to_string();
        std::string urgent = std::bitset<16>(0).to_string();
    
        std::string header = src + dst + seq + ack + offset_and_flags + window_size + checksum_placeholder + urgent;
    
        uint16_t checksum = computeChecksum(header);
        std::string checksum_bits = std::bitset<16>(checksum).to_string();
    
        std::string ack_packet = src + dst + seq + ack + offset_and_flags + window_size + checksum_bits + urgent;
    
       
    
        return ack_packet;
    }
    

    std::string Packet::FinPacket() {
        std::string src  = std::bitset<16>(source_port).to_string();
        std::string dst  = std::bitset<16>(dest_port).to_string();
    
        // Use current sequence number and increment after sending FIN
        std::string seq  = std::bitset<32>(last_seq_num).to_string();
        std::string ack  = std::bitset<32>(last_ack_num).to_string(); // optional, use if responding
    
        std::string data_offset = std::bitset<4>(5).to_string();
        std::string reserved    = std::bitset<3>(0).to_string();
        std::string flags       = std::bitset<9>(0x01).to_string(); // FIN
        std::string offset_and_flags = data_offset + reserved + flags;
    
        std::string window_size = std::bitset<16>(0x24).to_string();
        std::string checksum_placeholder = std::bitset<16>(0).to_string();
        std::string urgent = std::bitset<16>(0).to_string();
    
        std::string header = src + dst + seq + ack + offset_and_flags + window_size + checksum_placeholder + urgent;
    
        uint16_t checksum = computeChecksum(header);
        std::string checksum_bits = std::bitset<16>(checksum).to_string();
    
        std::string fin_packet = src + dst + seq + ack + offset_and_flags + window_size + checksum_bits + urgent;
    
        // TCP behavior: FIN consumes one sequence number
        last_seq_num += 1;
    
        return fin_packet;
    }
    
    std::string Packet::DataPacket(uint32_t seq_num, uint32_t ack_num, const std::string& payload) {
        std::string src  = std::bitset<16>(source_port).to_string();
        std::string dst  = std::bitset<16>(dest_port).to_string();
    
        if (seq_num == 0) seq_num = last_seq_num;  // Use last sequence number if not provided
        if (ack_num == 0) ack_num = last_ack_num;  // Use last ack if not provided
    
        std::string seq  = std::bitset<32>(seq_num).to_string();
        std::string ack  = std::bitset<32>(ack_num).to_string();
    
        std::string data_offset = std::bitset<4>(5).to_string();
        std::string reserved    = std::bitset<3>(0).to_string();
        std::string flags       = std::bitset<9>(0x10).to_string(); // ACK
        std::string offset_and_flags = data_offset + reserved + flags;
    
        std::string window_size = std::bitset<16>(0x24).to_string();
        std::string checksum_placeholder = std::bitset<16>(0).to_string();
        std::string urgent = std::bitset<16>(0).to_string();
    
        std::string header = src + dst + seq + ack + offset_and_flags + window_size + checksum_placeholder + urgent;
        std::string full_packet = header + payload;
    
        uint16_t checksum = computeChecksum(full_packet);
        std::string checksum_bits = std::bitset<16>(checksum).to_string();
    
        std::string final_packet = src + dst + seq + ack + offset_and_flags + window_size + checksum_bits + urgent + payload;
    
        // Update internal tracking
        last_seq_num = seq_num + payload.size() / 8;  // One byte per 8 bits of payload
        last_ack_num = ack_num;
    
        return final_packet;
    }
    
     

bool Packet::SendPacket(int socket_fd, const std::string& packet) {
    ssize_t sent = send(socket_fd, packet.c_str(), packet.size(), 0);
    return sent == (ssize_t)packet.size();
}

uint16_t Packet::computeChecksum(const std::string& bitstring) {
    uint32_t sum = 0;

    // Process the input 16 bits at a time
    for (size_t i = 0; i + 16 <= bitstring.size(); i += 16) {
        std::string chunk = bitstring.substr(i, 16);
        uint16_t word = std::bitset<16>(chunk).to_ulong();
        sum += word;

        // Wrap around if overflow
        if (sum > 0xFFFF) {
            sum = (sum & 0xFFFF) + 1;
        }
    }

    // If bitstring length isn't divisible by 16, pad with 0s and process the last chunk
    if (bitstring.size() % 16 != 0) {
        std::string last_chunk = bitstring.substr(bitstring.size() - (bitstring.size() % 16));
        last_chunk.append(16 - last_chunk.length(), '0');
        uint16_t word = std::bitset<16>(last_chunk).to_ulong();
        sum += word;

        if (sum > 0xFFFF) {
            sum = (sum & 0xFFFF) + 1;
        }
    }

    // Return one's complement
    return ~sum & 0xFFFF;
}

bool Packet::verifyChecksum(const std::string& packet) {
    if (packet.length() < 160) {
        std::cerr << "[Checksum] Packet too short! Must be at least 160 bits.\n";
        return false;
    }

    // Extract checksum bits from fixed position (112–127)
    std::string checksumBits = packet.substr(128, 16);
    if (checksumBits.find_first_not_of("01") != std::string::npos)
        return false;

    // Replace checksum with 0s for validation
    std::string data = packet.substr(0, 128) + std::string(16, '0') +
                      ((packet.length() > 160) ? packet.substr(160) : "");

    uint16_t received = std::bitset<16>(checksumBits).to_ulong();
    uint16_t calculated = computeChecksum(data);

    // Debug printout
    std::cout << "[VerifyChecksum] Packet Length:  " << packet.length() << " bits\n";
    std::cout << "[VerifyChecksum] Data Bits:      " << data << "\n";
    std::cout << "[VerifyChecksum] Checksum Bits:  " << checksumBits << "\n";
    std::cout << "[VerifyChecksum] Received (dec): " << received << "\n";
    std::cout << "[VerifyChecksum] Calculated:     " << calculated << "\n";
    std::cout << "[VerifyChecksum] Match?          " << (received == calculated ? "YES" : " NO") << "\n";

    return received == calculated;
}


void Packet::parsePacket(const std::string& packet) {
    if (packet.size() < 160) {
        std::cerr << "Packet too short! Expected 160-bit header.\n";
        return;
    }

    // Extract 160-bit header fields
    std::string src_bits       = packet.substr(0, 16);
    std::string dst_bits       = packet.substr(16, 16);
    std::string seq_bits       = packet.substr(32, 32);
    std::string ack_bits       = packet.substr(64, 32);
    std::string offset_flags   = packet.substr(96, 16);  // 4+3+9 bits
    std::string window_bits    = packet.substr(112, 16);
    std::string checksum_bits  = packet.substr(128, 16);
    std::string urgent_bits    = packet.substr(144, 16);
    std::string payload_bits   = (packet.size() > 160) ? packet.substr(160) : "";

    // Decode fields
    uint16_t src_port   = std::bitset<16>(src_bits).to_ulong();
    uint16_t dst_port   = std::bitset<16>(dst_bits).to_ulong();
    uint32_t seq_num    = std::bitset<32>(seq_bits).to_ulong();
    uint32_t ack_num    = std::bitset<32>(ack_bits).to_ulong();
    uint8_t data_offset = std::bitset<4>(offset_flags.substr(0, 4)).to_ulong();
    uint8_t reserved    = std::bitset<3>(offset_flags.substr(4, 3)).to_ulong();
    uint16_t flags      = std::bitset<9>(offset_flags.substr(7)).to_ulong();
    uint16_t window     = std::bitset<16>(window_bits).to_ulong();
    uint16_t checksum   = std::bitset<16>(checksum_bits).to_ulong();
    uint16_t urgent     = std::bitset<16>(urgent_bits).to_ulong();

    // Update internal state
    last_seq_num = seq_num;
    last_ack_num = ack_num;

    // Flag decoding
    bool isFIN = flags & 0x01;
    bool isSYN = flags & 0x02;
    bool isRST = flags & 0x04;
    bool isPSH = flags & 0x08;
    bool isACK = flags & 0x10;
    bool isURG = flags & 0x20;

    // Display breakdown
    std::cout << "\n[Packet Breakdown]\n";
    std::cout << "Source Port     : " << src_port << " (" << src_bits << ")\n";
    std::cout << "Destination Port: " << dst_port << " (" << dst_bits << ")\n";
    std::cout << "Sequence Number : " << seq_num  << " (" << seq_bits << ")\n";
    std::cout << "Acknowledgment  : " << ack_num  << " (" << ack_bits << ")\n";
    std::cout << "Data Offset     : " << static_cast<int>(data_offset) << " (" << offset_flags.substr(0, 4) << ")\n";
    std::cout << "Reserved Bits   : " << static_cast<int>(reserved) << " (" << offset_flags.substr(4, 3) << ")\n";
    std::cout << "Flags           : " << offset_flags.substr(7) << " → ";
    if (isFIN) std::cout << "FIN ";
    if (isSYN) std::cout << "SYN ";
    if (isRST) std::cout << "RST ";
    if (isPSH) std::cout << "PSH ";
    if (isACK) std::cout << "ACK ";
    if (isURG) std::cout << "URG ";
    std::cout << "\n";
    std::cout << "Window Size     : " << window  << " (" << window_bits << ")\n";
    std::cout << "Checksum        : " << checksum << " (" << checksum_bits << ")\n";
    std::cout << "Urgent Pointer  : " << urgent  << " (" << urgent_bits << ")\n";

    if (!payload_bits.empty()) {
        std::cout << "Payload Bits    : " << payload_bits << "\n";
        std::cout << "Payload Size    : " << payload_bits.size() << " bits (" << payload_bits.size() / 8.0 << " bytes)\n";
    } else {
        std::cout << "Payload         : (none)\n";
    }

    std::cout << "─────────────────────────────────────────────\n";
}

