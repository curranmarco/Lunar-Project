#include "Socket/socket_client.hpp"
#include "Packet/packet_functions.hpp"

#include <iostream>
#include <string>
#include <bitset>
#include <cstring>
#include <unistd.h>

#define FLAG_EARTH 0x40
#define FLAG_REQUEST_SENSOR 0x80
#define FLAG_REQUEST_ACTUATOR 0x20
#define FLAG_FIN 0x01

std::string extractFlags(const std::string& packet) {
    size_t flags_start = 16 + 16 + 32 + 32 + 4 + 3;
    return packet.substr(flags_start, 9);
}

void sendRequest(SocketClient& client, Packet& packet, uint8_t flag) {
    std::string payload(32, '0');  // Empty payload for command
    std::string request = packet.DataPacket(0, 0, payload, flag);
    packet.SendPacket(client.getSocket(), request);
    std::cout << "[EARTH] Command sent to server. Waiting for response...\n";

    char buffer[2048];
    memset(buffer, 0, sizeof(buffer));
    int len = recv(client.getSocket(), buffer, sizeof(buffer), 0);
    if (len > 0) {
        std::string response(buffer, len);
        if (packet.verifyChecksum(response)) {
            std::cout << "[EARTH] 🌐 Received response from server:\n" << response << "\n";
        } else {
            std::cerr << "[EARTH] ❌ Invalid checksum in response.\n";
        }
    } else {
        std::cerr << "[EARTH] Failed to receive data or connection closed.\n";
    }
}

int main() {
    SocketClient client("127.0.0.1", 8080);
    if (!client.connectToServer()) return 1;

    int sock = client.getSocket();
    Packet packet(1236, 8080);
    char buffer[2048];

    // --- Handshake ---
    std::string syn = packet.SynPacket();
    packet.SendPacket(sock, syn);
    std::cout << "[EARTH] Sent SYN.\n";

    memset(buffer, 0, sizeof(buffer));
    int bytes_received = recv(sock, buffer, sizeof(buffer), 0);
    std::string synack(buffer, bytes_received);
    std::cout << "[EARTH] Received SYN-ACK.\n";

    if (!packet.verifyChecksum(synack)) {
        std::cerr << "[EARTH] Invalid SYN-ACK checksum.\n";
        return 1;
    }

    std::string seq_bits = synack.substr(16 + 16, 32);
    uint32_t server_seq = std::bitset<32>(seq_bits).to_ulong();

    std::string ack = packet.AckPacket(server_seq + 1);
    packet.SendPacket(sock, ack);
    std::cout << "[EARTH] Sent ACK.\n";

    // --- Identification as EARTH ---
    std::string identity = packet.DataPacket(0, 0, std::string(32, '0'), FLAG_EARTH);
    packet.SendPacket(sock, identity);
    std::cout << "[EARTH] Sent identity packet.\n";

    // --- Command loop ---
    while (true) {
        std::cout << "\n🪐 Earth Command Center\n";
        std::cout << "1. Request sensor data\n";
        std::cout << "2. Request actuator speed\n";
        std::cout << "0. Exit\n";
        std::cout << "> ";
        int option;
        std::cin >> option;

        if (option == 0) {
            std::cout << "[EARTH] Closing connection.\n";
            break;
        } else if (option == 1) {
            sendRequest(client, packet, FLAG_REQUEST_SENSOR);
        } else if (option == 2) {
            sendRequest(client, packet, FLAG_REQUEST_ACTUATOR);
        } else {
            std::cout << "Invalid option.\n";
        }
    }

    client.closeConnection();
    return 0;
}
