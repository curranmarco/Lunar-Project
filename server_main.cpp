#include "Socket/socket_server.hpp"
#include "Packet/packet_functions.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>

int main() {
    srand(time(NULL));
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
    std::string synack = packet.SynAckPacket(0); // You could extract seq from SYN
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
   // --- 4. Receive and ACK Data Packets ---
std::cout << "[Server] Type 'fin' to send FIN to client and begin connection teardown.\n";

int dropped_count = 0;
const int max_drops = 5;
bool enable_dropping = true;

while (true) {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    FD_SET(client_socket, &readfds);

    int max_fd = std::max(STDIN_FILENO, client_socket) + 1;
    int activity = select(max_fd, &readfds, nullptr, nullptr, nullptr);

    if (activity < 0) {
        std::cerr << "[Server] select() error.\n";
        break;
    }

    // --- Terminal input handling
    if (FD_ISSET(STDIN_FILENO, &readfds)) {
        std::string input;
        std::getline(std::cin, input);

        if (input == "fin") {
            // 4-way teardown as before
            std::string server_fin = packet.FinPacket();
            packet.SendPacket(client_socket, server_fin);
            std::cout << "[Server] Sent FIN.\n";

            memset(buffer, 0, sizeof(buffer));
            int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
            std::string ack_of_fin(buffer, bytes_received);
            std::cout << "[Server] Received ACK for FIN:\n" << ack_of_fin << "\n";

            if (!packet.verifyChecksum(ack_of_fin)) {
                std::cerr << "[Server] Invalid ACK for FIN.\n";
                break;
            }

            memset(buffer, 0, sizeof(buffer));
            bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
            std::string client_fin(buffer, bytes_received);
            std::cout << "[Server] Received FIN from client:\n" << client_fin << "\n";

            if (!packet.verifyChecksum(client_fin)) {
                std::cerr << "[Server] Client FIN checksum invalid.\n";
                break;
            }

            std::string final_ack = packet.AckPacket(0);
            packet.SendPacket(client_socket, final_ack);
            std::cout << "[Server] Sent final ACK.\n";
            break;
        } else if (input == "5") {
            dropped_count = 0;
            enable_dropping = true;
            std::cout << "[Server] 🔄 Packet drop counter reset. Dropping resumes.\n";
        }
    }

    // --- Client data packet handling
    if (FD_ISSET(client_socket, &readfds)) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            std::cerr << "[Server] Client disconnected or error.\n";
            break;
        }

        std::string data_packet(buffer, bytes_received);
        std::cout << "[Server] Received DATA:\n" << data_packet << "\n";

        if (!packet.verifyChecksum(data_packet)) {
            std::cerr << "[Server] Invalid DATA checksum.\n";
            continue;
        }

        // 💣 Simulate packet loss up to 5 times
        if (enable_dropping && dropped_count < max_drops) {
            int dropChance = rand() % 100;
            if (dropChance < 50) { // 50% chance to drop (adjustable)
                dropped_count++;
                std::cout << "[Server] ⚠️ Dropping packet " << dropped_count << " of " << max_drops << "\n\n";
                continue; // Simulate loss
            }
        }

        // ✅ Send ACK normally
        std::string ack_response = packet.AckPacket(0);
        packet.SendPacket(client_socket, ack_response);
        std::cout << "[Server] Sent ACK.\n\n";
    }
}


   
    // --- Done ---
    server.closeConnection();
    std::cout << "[Server] Connection closed cleanly.\n";
    return 0;
}


