#include "Socket/socket_server.hpp"
#include "Packet/packet_functions.hpp"

#include <iostream>
#include <bitset>
#include <string>
#include <cstring>
#include <unistd.h>
#include <thread>
#include <chrono>

#define FLAG_MOVE_STOP 0x08
#define FLAG_SPEED_REQ 0x20

void sendCommand(int client_socket, uint16_t flag, uint16_t src_port, uint16_t dest_port, const std::string& data) {
    Packet packet(src_port, dest_port);
    uint32_t seq = rand();
    uint32_t ack = 0;
    std::string cmd = packet.DataPacket(seq, ack, data, flag);

    if (packet.SendPacket(client_socket, cmd)) {
        std::cout << "\n📤 Command packet sent (flag: " << std::bitset<9>(flag) << ")\n";
    } else {
        std::cerr << "❌ Failed to send command packet.\n";
    }
}

void listenForResponse(int client_socket, bool expectSpeed) {
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    ssize_t bytes_received = read(client_socket, buffer, sizeof(buffer));
    if (bytes_received <= 0) {
        std::cerr << "❌ No data received.\n";
        return;
    }

    std::string received_packet(buffer, bytes_received);
    std::cout << "\n📩 Response packet received:\n" << received_packet << "\n";

    if (!Packet::verifyChecksum(received_packet)) {
        std::cerr << "❌ Invalid checksum.\n";
        return;
    }
    
    std::cout << "✅ Checksum valid.\n";

    size_t payload_start = 16 + 1 + 16 + 1 + 32 + 1 + 32 + 1 + 32 + 1 + 32 + 1;
    std::string payload = received_packet.substr(payload_start);
    size_t space_pos = payload.find(' ');
    if (space_pos != std::string::npos) {
        payload = payload.substr(0, space_pos);
    }

    std::cout << "📦 Payload: " << payload << "\n";

    if (expectSpeed && payload.length() >= 32) {
        std::bitset<8> speed_bits(payload.substr(24, 8));
        int speed_value = static_cast<int>(speed_bits.to_ulong());
        std::cout << "🚀 Speed value extracted: " << speed_value << "\n";
    }
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    int port = 8080;
    SocketServer server(port);

    if (!server.startListening()) return 1;
    if (!server.acceptClient()) return 1;

    int client_socket = server.getClientSocket();

    uint16_t server_port = port;
    uint16_t client_port = 5000;

    while (true) {
        int choice;
        std::cout << "\n🕹️ Choose command to send:\n";
        std::cout << "1. MOVE\n";
        std::cout << "2. STOP\n";
        std::cout << "3. REQUEST SPEED\n";
        std::cout << "0. Exit\n> ";
        std::cin >> choice;

        if (choice == 0) break;

        switch (choice) {
            case 1: {
                std::string move_payload = "00000001" + std::string(24, '0');
                sendCommand(client_socket, FLAG_MOVE_STOP, server_port, client_port, move_payload);
                listenForResponse(client_socket, false);
                break;
            }
            case 2: {
                std::string stop_payload = "00000000" + std::string(24, '0');
                sendCommand(client_socket, FLAG_MOVE_STOP, server_port, client_port, stop_payload);
                listenForResponse(client_socket, false);
                break;
            }
            case 3: {
                std::string dummy_payload(32, '0');
                sendCommand(client_socket, FLAG_SPEED_REQ, server_port, client_port, dummy_payload);
                listenForResponse(client_socket, true);
                break;
            }
            default:
                std::cout << "Invalid choice.\n";
                continue;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    return 0;
}