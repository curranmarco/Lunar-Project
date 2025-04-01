// This is a new client_main.cpp adapted for actuator functionality.
// Replaces sensor logic with a continuous listener that responds to commands from server.

#include "Packet/packet_functions.hpp"
#include "Socket/socket_client.hpp"

#include <iostream>
#include <bitset>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>
#include <sstream>

#define FLAG_MOVE_STOP "000001000"
#define FLAG_SPEED_REQ "000100000"

std::string generateDataPayload(bool isMoving, int speed = -1) {
    if (speed != -1) {
        std::bitset<8> speed_bits(speed);
        return std::string(24, '0') + speed_bits.to_string();
    }
    return isMoving ? std::string(32, '1') : std::string(32, '0');
}

std::string extractFlags(const std::string& packet) {
    std::istringstream ss(packet);
    std::string field;
    int count = 0;
    while (std::getline(ss, field, ' ')) {
        if (count == 4) {
            return field.substr(7, 9); // fixed: correct 9-bit flag slice
        }
        count++;
    }
    return "";
}

std::string extractPayload(const std::string& packet) {
    std::istringstream ss(packet);
    std::string field;
    int count = 0;
    while (std::getline(ss, field, ' ')) {
        if (count >= 6) return field;
        count++;
    }
    return "";
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));

    std::string server_ip = "127.0.0.1";
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

    char buffer[1024];

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

        if (flags == FLAG_MOVE_STOP) {
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

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    return 0;
}
