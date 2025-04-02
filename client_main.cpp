// This is a new client_main.cpp adapted for actuator functionality.
// Replaces sensor logic with a continuous listener that responds to commands from server.

#include "Packet/packet_functions.hpp"
#include "Socket/socket_client.hpp"
#include "Socket/socket_server.hpp"

#include <iostream>
#include <bitset>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>
#include <sstream>

#define FLAG_MOVE_STOP "000001000"
#define FLAG_SPEED_REQ "000100000"
#define FLAG_FIN "000000001"


std::string generateDataPayload(bool isMoving, int speed = -1) {
    if (speed != -1) {
        std::bitset<8> speed_bits(speed);
        return std::string(24, '0') + speed_bits.to_string();
    }
    return isMoving ? std::string(32, '1') : std::string(32, '0');
}

std::string extractFlags(const std::string& packet) {
    size_t flags_start = 16 + 16 + 32 + 32 + 4 + 3;
    return packet.substr(flags_start, 9);
}

std::string extractPayload(const std::string& packet) {
    size_t payload_start = 16 + 16 + 32 + 32 + 4 + 3 + 9 + 16 + 16 + 16;
    return packet.substr(payload_start);
}

void peerServerThread(Packet& packet, int server_socket, uint32_t& seq) {
    SocketServer peer_server(9000);
    peer_server.startListening();
    std::cout << "📡 [Peer Server] Listening for peer connections on port 9000...\n";
    peer_server.acceptClient();

    int peer_sock = peer_server.getClientSocket();
    char buf[1024] = {0};
    while (true) {
        ssize_t bytes = recv(peer_sock, buf, sizeof(buf), 0);
        if (bytes > 0) {
            std::string msg(buf, bytes);
            std::cout << "\n🔄 [Peer] Received from other client: " << msg << "\n";
            // 👉 Prepare STOP command payload
            std::string stop_payload = "00000000" + std::string(24, '0'); // MOVE flag OFF, speed zero
            std::string stop_packet = packet.DataPacket(seq++, 0, stop_payload, 0x10); // flag 0x10 = ACK

            // 👉 Send it to server
            if (packet.SendPacket(server_socket, stop_packet)) {
                std::cout << "🛑 Sent STOP command to server in response to peer.\n";
            } else {
                std::cerr << "❌ Failed to send STOP packet to server.\n";
            }
        }
    }
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));

    std::string server_ip = "192.168.221.1";
    int server_port = 8080;
    uint16_t source_port = 0;
    uint16_t dest_port = 8080;

    SocketClient client(server_ip, server_port);
    if (!client.connectToServer(source_port)) {
        std::cerr << "❌ Connection to server failed.\n";
        return 1;
    }

    Packet packet(source_port, dest_port);
    uint32_t seq = rand();

    std::thread peer_thread(peerServerThread, std::ref(packet), client.getSocket(), std::ref(seq));
    peer_thread.detach();

    // --- Send SYN Packet ---
    std::string syn = packet.SynPacket();
    packet.SendPacket(client.getSocket(), syn);
    std::cout << "[Client] Sent SYN:\n" << syn << "\n\n";
    //packet.SendPacket(client.getSocket(), packet.DataPacket(seq, 0, "11111111", 0x02));

    char buffer[1024];
    // --- Receive SYN-ACK and send final ACK ---
    memset(buffer, 0, sizeof(buffer));
    ssize_t synack_received = recv(client.getSocket(), buffer, sizeof(buffer), 0);
    if (synack_received <= 0) {
        std::cerr << "❌ Did not receive SYN-ACK.\n";
        return 1;
    }
    std::string synack_packet(buffer, synack_received);
    std::cout << "[Client] Received SYN-ACK:\n" << synack_packet << "\n";
    if (!Packet::verifyChecksum(synack_packet)) {
        std::cerr << "❌ Invalid SYN-ACK checksum.\n";
        return 1;
    }

    std::string ack = packet.AckPacket(0);
    packet.SendPacket(client.getSocket(), ack);
    std::cout << "[Client] Sent ACK for SYN-ACK:\n" << ack << "\n";

    std::cout << "🚀 Actuator client started. Waiting for instructions...\n";

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytes_received = recv(client.getSocket(), buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            std::cerr << "❌ Connection lost or no data received.\n";
            break;
        }

        std::string received_packet(buffer, bytes_received);
        std::cout << "\n📦 Packet received.\n";
        std::cout << "[DEBUG] Full packet: " << received_packet << "\n";

        if (!Packet::verifyChecksum(received_packet)) {
            std::cerr << "❌ Invalid checksum.\n";
            continue;
        }

        std::string flags = extractFlags(received_packet);
        std::cout << "[DEBUG] Extracted flags: " << flags << "\n";

        std::string response_data;
        if (flags == FLAG_FIN){
            std::cout << "📴 Received FIN from server. Closing connection.\n";
            std::string ack = packet.AckPacket(0);
            packet.SendPacket(client.getSocket(), ack);
            std::cout << "📬 Sent ACK for FIN.\n";
            break;
        }
        else if (flags == FLAG_MOVE_STOP) {
            std::string payload = extractPayload(received_packet);
            bool move = payload.substr(0, 8) == "00000001";
            std::cout << "🔧 Executing command: " << (move ? "MOVE" : "STOP") << "\n";
            response_data = generateDataPayload(move);

        } else if (flags == FLAG_SPEED_REQ) {
            int speed = (rand() % 128) + 1;
            response_data = generateDataPayload(false, speed);
            std::cout << "🚦 Sending speed: " << speed << "\n";

        } else {
            std::cout << "⚠️ Unknown command flag received: [" << flags << "]\n";
            continue;
        }

        std::string response_packet = packet.DataPacket(seq, 0, response_data, 0x10);
        packet.SendPacket(client.getSocket(), response_packet);
        seq += response_data.length();
        bool ack_received = false;
        for (int attempts = 0; attempts < 3 && !ack_received; ++attempts) {
            fd_set readfds;
            struct timeval timeout;
            FD_ZERO(&readfds);
            FD_SET(client.getSocket(), &readfds);
            timeout.tv_sec = 2;  // wait up to 2 seconds
            timeout.tv_usec = 0;

            int result = select(client.getSocket() + 1, &readfds, nullptr, nullptr, &timeout);
            if (result > 0 && FD_ISSET(client.getSocket(), &readfds)) {
                memset(buffer, 0, sizeof(buffer));
                ssize_t ack_bytes = recv(client.getSocket(), buffer, sizeof(buffer), 0);
                std::string ack_response(buffer, ack_bytes);
                if (Packet::verifyChecksum(ack_response)) {
                    std::cout << "✅ ACK received after data.\n";
                    ack_received = true;
                }
            } else {
                std::cout << "⏱️ No ACK received, retrying (" << attempts + 1 << "/3)...\n";
                packet.SendPacket(client.getSocket(), response_packet);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    return 0;
}