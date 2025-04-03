#include "Socket/socket_server.hpp"
#include "Packet/packet_functions.hpp"

#include <iostream>
#include <bitset>
#include <string>
#include <thread>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

#define FLAG_SENSOR 0x04
#define FLAG_ACTUATOR 0x08
#define FLAG_EARTH 0x40
#define FLAG_REQUEST_SENSOR 0x80
#define FLAG_REQUEST_ACTUATOR 0x20
#define FLAG_ACK 0x10
#define FLAG_FIN 0x01

std::string extractFlags(const std::string& packet) {
    return packet.substr(16 + 16 + 32 + 32 + 4 + 3, 9);
}

int actuatorSock = -1;
int sensorSock = -1;
int earthSock = -1;
bool actuatorReady = false;
bool sensorReady = false;
bool earthReady = false;

Packet global_packet(8080, 5000);

void forwardAndRelayResponse(int fromSock, int toSock, bool patchToAck = false) {
    char buffer[2048] = {0};
    ssize_t len = recv(fromSock, buffer, sizeof(buffer), 0);
    if (len > 0) {
        std::string response(buffer, len);
        if (Packet::verifyChecksum(response)) {
            // Send ACK to client
            std::string ack = global_packet.AckPacket(0);
            global_packet.SendPacket(fromSock, ack);
            std::cout << "✅ Sent ACK back to data sender.\n";

            if (patchToAck) {
                std::string patched = response;
                size_t flag_index = 16 + 16 + 32 + 32 + 4 + 3;
                patched.replace(flag_index, 9, std::bitset<9>(FLAG_ACK).to_string());
                global_packet.SendPacket(toSock, patched);
            } else {
                global_packet.SendPacket(toSock, response);
            }
        } else {
            std::cerr << "❌ Invalid checksum in response." << std::endl;
        }
    } else {
        std::cerr << "❌ Failed to receive data." << std::endl;
    }
}

void handleEarth() {
    std::cout << "🌍 Earth interface ready.\n";
    char buffer[2048];

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t len = recv(earthSock, buffer, sizeof(buffer), 0);
        if (len <= 0) break;

        std::string command(buffer, len);
        if (!Packet::verifyChecksum(command)) continue;

        std::string flags = extractFlags(command);
        uint16_t flag_val = std::bitset<9>(flags).to_ulong();

        if (flag_val == FLAG_REQUEST_SENSOR && sensorReady) {
            global_packet.SendPacket(sensorSock, command);
            forwardAndRelayResponse(sensorSock, earthSock, true); // patch sensor -> ack
        } else if (flag_val == FLAG_REQUEST_ACTUATOR && actuatorReady) {
            global_packet.SendPacket(actuatorSock, command);
            forwardAndRelayResponse(actuatorSock, earthSock);
        } else {
            std::cerr << "⚠️ Unknown or unready client for command." << std::endl;
        }
    }
}

void handleClient(int sock) {
    char buffer[2048] = {0};

    recv(sock, buffer, sizeof(buffer), 0);
    std::string syn(buffer);
    std::cout << "📥 Received SYN:\n" << syn << std::endl;
    if (!Packet::verifyChecksum(syn)) return;

    std::string seq_bits = syn.substr(32, 32);
    uint32_t seq = std::bitset<32>(seq_bits).to_ulong();
    std::string synack = global_packet.SynAckPacket(seq);
    global_packet.SendPacket(sock, synack);
    std::cout << "📤 Sent SYN-ACK" << std::endl;

    memset(buffer, 0, sizeof(buffer));
    recv(sock, buffer, sizeof(buffer), 0);
    std::string ack(buffer);
    if (!Packet::verifyChecksum(ack)) return;
    std::cout << "✅ Handshake complete" << std::endl;

    memset(buffer, 0, sizeof(buffer));
    recv(sock, buffer, sizeof(buffer), 0);
    std::string identity(buffer);
    std::string flags = extractFlags(identity);
    uint16_t role = std::bitset<9>(flags).to_ulong();

    if (role == FLAG_ACTUATOR) {
        actuatorSock = sock;
        actuatorReady = true;
        std::cout << "🔧 Client is ACTUATOR" << std::endl;
    } else if (role == FLAG_SENSOR) {
        sensorSock = sock;
        sensorReady = true;
        std::cout << "🌡️ Client is SENSOR" << std::endl;
    } else if (role == FLAG_EARTH) {
        earthSock = sock;
        earthReady = true;
        std::cout << "🌍 Client is EARTH" << std::endl;
        std::thread earthThread(handleEarth);
        earthThread.detach();
        return;
    }

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t len = recv(sock, buffer, sizeof(buffer), 0);
        if (len <= 0) break;
        std::string msg(buffer, len);
        if (Packet::verifyChecksum(msg)) {
            std::cout << "[VerifyChecksum] Packet Length:  " << msg.length() << " bits\n";
            std::cout << "[VerifyChecksum] Match?          YES\n";
        }
    }
}

void serverMain() {
    int port = 8080;
    SocketServer server(port);
    if (!server.startListening()) return;

    std::cout << "🚪 Server is running on port " << port << std::endl;

    while (true) {
        if (!server.acceptClient()) continue;
        int sock = server.getClientSocket();
        std::cout << "🔌 New client connected." << std::endl;
        std::thread clientThread(handleClient, sock);
        clientThread.detach();
    }
}

int main() {
    serverMain();
    return 0;
}
