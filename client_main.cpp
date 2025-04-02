#include "Packet/packet_functions.hpp"
#include "Socket/socket_client.hpp"

#include <iostream>
#include <bitset>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>

#define FLAG_MOVE_STOP 0x08
#define FLAG_SPEED_REQ 0x20
#define FLAG_ACK 0x10
#define FLAG_FIN 0x01

std::string generateDataPayload(bool isMoving, int speed = -1) {
    if (speed != -1) {
        std::bitset<8> speed_bits(speed);
        return std::string(24, '0') + speed_bits.to_string();
    }
    return isMoving ? std::string(32, '1') : std::string(32, '0');
}

std::string extractFlags(const std::string& packet) {
    return packet.substr(16 + 16 + 32 + 32 + 4 + 3, 9);
}

std::string extractPayload(const std::string& packet) {
    size_t start = 16 + 16 + 32 + 32 + 4 + 3 + 9 + 16 + 16 + 16;
    return packet.substr(start);
}

int main() {
    srand(time(0));
    SocketClient client("127.0.0.1", 8080);
    if (!client.connectToServer()) {
        std::cerr << "❌ Could not connect to server.\n";
        return 1;
    }

    int sock = client.getSocket();
    Packet packet(1234, 8080);
    uint32_t seq = rand();
    char buffer[2048] = {0};

    std::string syn = packet.SynPacket();
    packet.SendPacket(sock, syn);
    ssize_t synack_len = recv(sock, buffer, sizeof(buffer), 0);
    std::string synack(buffer, synack_len);
    if (!Packet::verifyChecksum(synack)) return 1;

    uint32_t server_seq = std::bitset<32>(synack.substr(32, 32)).to_ulong();
    std::string ack = packet.AckPacket(server_seq + 1);
    packet.SendPacket(sock, ack);

    std::string init = packet.DataPacket(seq++, 0, std::string(32, '0'), FLAG_MOVE_STOP);
    packet.SendPacket(sock, init);
    std::cout << "Actuator client started. Waiting for instructions...\n";

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t len = recv(sock, buffer, sizeof(buffer), 0);
        if (len <= 0) break;

        std::string pkt(buffer, len);
        if (!Packet::verifyChecksum(pkt)) continue;

        std::string flags = extractFlags(pkt);
        std::string payload = extractPayload(pkt);
        uint16_t flag_val = std::bitset<9>(flags).to_ulong();

        std::string response;

        if (flag_val == FLAG_MOVE_STOP) {
            bool move = (payload.substr(0, 8) == "00000001");
            std::cout << (move ? "🚨 Executing: MOVE\n" : "Executing: STOP\n");
            response = generateDataPayload(move);
        } else if (flag_val == FLAG_SPEED_REQ) {
            int speed = rand() % 128;
            std::cout << "Sending speed: " << speed << "\n";
            response = generateDataPayload(false, speed);
        } else if (flag_val == FLAG_FIN) {
            std::cout << "Received FIN. Closing connection.\n";
            std::string ack_fin = packet.AckPacket(0);
            packet.SendPacket(sock, ack_fin);
            break;
        }

        std::string response_pkt = packet.DataPacket(seq++, 0, response, FLAG_ACK);
        packet.SendPacket(sock, response_pkt);

        bool acked = false;
        for (int retry = 0; retry < 3 && !acked; retry++) {
            fd_set fds;
            FD_ZERO(&fds); FD_SET(sock, &fds);
            struct timeval timeout = {2, 0};
            if (select(sock + 1, &fds, nullptr, nullptr, &timeout) > 0) {
                memset(buffer, 0, sizeof(buffer));
                ssize_t ack_len = recv(sock, buffer, sizeof(buffer), 0);
                if (ack_len > 0 && Packet::verifyChecksum(std::string(buffer, ack_len))) {
                    std::cout << "ACK received.\n";
                    acked = true;
                }
            }
            if (!acked) {
                std::cout << "Resending packet (retry " << retry + 1 << ")\n";
                packet.SendPacket(sock, response_pkt);
            }
        }
    }

    client.closeConnection();
    return 0;
}
