#include "Socket/socket_server.hpp"
#include "Packet/packet_functions.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>

int main() {
    SocketServer server(8080);
    if (!server.startListening() || !server.acceptClient()) {
        return 1;
    }

    int client_socket = server.getClientSocket();
    Packet packet(8080, 1234);
    char buffer[4096] = {0};

    // --- 1. Wait for SYN ---
    int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    std::string syn(buffer, bytes_received);
    std::cout << "[Server] Received SYN:\n" << syn << "\n";

    if (!packet.verifyChecksum(syn)) {
        std::cerr << "[Server] Invalid SYN checksum.\n";
        server.closeConnection();
        return 1;
    }

    packet.parsePacket(syn);

    // --- 2. Send SYN-ACK ---
    std::string synack = packet.SynAckPacket(11);  // Use client seq + 1 if available
    packet.SendPacket(client_socket, synack);
    std::cout << "[Server] Sent SYN-ACK:\n" << synack << "\n";

    // --- 3. Wait for ACK ---
    memset(buffer, 0, sizeof(buffer));
    bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    std::string ack(buffer, bytes_received);
    std::cout << "[Server] Received ACK:\n" << ack << "\n";

    if (!packet.verifyChecksum(ack)) {
        std::cerr << "[Server] Invalid ACK checksum.\n";
        server.closeConnection();
        return 1;
    }

    packet.parsePacket(ack);
    std::cout << "[Server] Handshake complete. Listening for data...\n\n";

    // --- Main Loop ---
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes <= 0) {
            std::cerr << "[Server] Connection closed or error.\n";
            break;
        }

        std::string recv_packet(buffer, bytes);
        if (!packet.verifyChecksum(recv_packet)) {
            std::cerr << "[Server] Invalid packet checksum. Dropping...\n";
            continue;
        }

        packet.parsePacket(recv_packet);

        // Extract flags to handle FIN, DATA, etc.
        std::string flags_bits = recv_packet.substr(103, 9);
        int flags = std::bitset<9>(flags_bits).to_ulong();

        if (flags & 0x01) {  // FIN
            std::cout << "[Server] Received FIN. Starting teardown...\n";

            // Step 1: ACK client's FIN
            std::string ack_for_fin = packet.AckPacket(packet.last_seq_num);
            packet.SendPacket(client_socket, ack_for_fin);
            std::cout << "[Server] Sent ACK for FIN.\n";

            // Step 2: Send server FIN
            std::string server_fin = packet.FinPacket();
            packet.SendPacket(client_socket, server_fin);
            std::cout << "[Server] Sent FIN.\n";

            // Step 3: Wait for final ACK from client
            memset(buffer, 0, sizeof(buffer));
            int ack_fin = recv(client_socket, buffer, sizeof(buffer), 0);
            if (ack_fin > 0) {
                std::string final_ack(buffer, ack_fin);
                std::cout << "[Server] Final ACK:\n" << final_ack << "\n";
            }

            break;
        }
        else if (flags & 0x08) {  // PSH/Data
            std::cout << "[Server] Received DATA. Sending ACK...\n";
            std::string ack_data = packet.AckPacket(packet.last_seq_num);
            packet.SendPacket(client_socket, ack_data);
        }
        else {
            std::cout << "[Server] Received packet with unknown flags.\n";
        }
    }

    server.closeConnection();
    std::cout << "[Server] Connection closed.\n";
    return 0;
}



