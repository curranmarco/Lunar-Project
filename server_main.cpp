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
    Packet packet(8080, 1234);  // Server's port, then client port

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

    // --- 2. Send SYN-ACK ---
    std::string synack = packet.SynAckPacket(0);
    packet.SendPacket(client_socket, synack);
    std::cout << "[Server] Sent SYN-ACK:\n" << synack << "\n";

    // --- 3. Wait for final ACK ---
    memset(buffer, 0, sizeof(buffer));
    bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    std::string ack(buffer, bytes_received);
    std::cout << "[Server] Received ACK:\n" << ack << "\n";

    if (!packet.verifyChecksum(ack)) {
        std::cerr << "[Server] Invalid ACK checksum.\n";
        server.closeConnection();
        return 1;
    }

    std::cout << "[Server] Handshake complete.\n\n";

    // --- 4. Receive and ACK Data Packets ---
    int packet_count = 0;
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            std::cerr << "[Server] Client disconnected or error.\n";
            break;
        }

        std::string data_packet(buffer, bytes_received);
        std::cout << "[Server] Received DATA:\n" << data_packet << "\n";

        if (!packet.verifyChecksum(data_packet)) {
            std::cerr << "[Server] Invalid DATA checksum.\n";
            break;
        }

        std::string ack_response = packet.AckPacket(0);
        packet.SendPacket(client_socket, ack_response);
        std::cout << "[Server] Sent ACK.\n\n";

        packet_count++;
        if (packet_count >= 5) break;
    }

    // --- Begin Teardown (Server-Initiated) ---
    std::string server_fin = packet.FinPacket();
    packet.SendPacket(client_socket, server_fin);
    std::cout << "[Server] Sent FIN.\n";

    // --- Wait for ACK of server FIN ---
    memset(buffer, 0, sizeof(buffer));
    bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    std::string ack_of_fin(buffer, bytes_received);
    std::cout << "[Server] Received ACK for FIN:\n" << ack_of_fin << "\n";

    if (!packet.verifyChecksum(ack_of_fin)) {
        std::cerr << "[Server] Invalid ACK for FIN.\n";
        server.closeConnection();
        return 1;
    }

    // --- Wait for client FIN ---
    memset(buffer, 0, sizeof(buffer));
    bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    std::string client_fin(buffer, bytes_received);
    std::cout << "[Server] Received FIN from client:\n" << client_fin << "\n";

    if (!packet.verifyChecksum(client_fin)) {
        std::cerr << "[Server] Client FIN checksum invalid.\n";
        server.closeConnection();
        return 1;
    }

    // --- Final ACK to client FIN ---
    std::string final_ack = packet.AckPacket(0);
    packet.SendPacket(client_socket, final_ack);
    std::cout << "[Server] Sent final ACK.\n";

    server.closeConnection();
    std::cout << "[Server] Connection closed after four-way teardown.\n";
    return 0;
}

