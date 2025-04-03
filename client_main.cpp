#include "Socket/socket_client.hpp"
#include "Packet/packet_functions.hpp"
#include "Sensors/Sensors.hpp"

#include <iostream>
#include <unistd.h>
#include <cstring>
#include <bitset>
#include <chrono>
#include <thread>

#define FLAG_SENSOR 0x04
#define FLAG_ACK 0x10
#define FLAG_FIN 0x01
#define FLAG_DATA_REQUEST 0x80  // assuming request flag from server

std::string extractFlags(const std::string& packet) {
    size_t flags_start = 16 + 16 + 32 + 32 + 4 + 3;
    return packet.substr(flags_start, 9);
}

int main() {
    SocketClient client("127.0.0.1", 8080);
    if (!client.connectToServer()) return 1;

    int sock = client.getSocket();
    Packet packet(1234, 8080);
    u_int32_t seq = rand(); 
    char buffer[4096] = {0};

    // --- Handshake ---
    std::string syn = packet.SynPacket();
    packet.SendPacket(sock, syn);
    std::cout << "[Client] Sent SYN:\n" << syn << "\n\n";

    int bytes_received = recv(sock, buffer, sizeof(buffer), 0);
    std::string synack(buffer, bytes_received);
    std::cout << "[Client] Received SYN-ACK:\n" << synack << "\n\n";

    if (!packet.verifyChecksum(synack)) {
        std::cerr << "[Client] Invalid SYN-ACK checksum.\n";
        return 1;
    }

    std::string seq_bits = synack.substr(16 + 16, 32);
    uint32_t server_seq = std::bitset<32>(seq_bits).to_ulong();
    std::string ack = packet.AckPacket(server_seq + 1);
    packet.SendPacket(sock, ack);
    std::cout << "[Client] Sent ACK.\n[Client] Handshake complete.\n\n";

    // --- Identify as SENSOR ---
    std::string init_packet = packet.DataPacket(seq++, 0, std::string(32, '0'), FLAG_SENSOR);
    packet.SendPacket(sock, init_packet);
    std::cout << "[Client] Sent role-identification packet (SENSOR).\n";

    Sensor moistureSensor("MoistureSensor", 0, 100);
    uint32_t seq_num = 0;

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(sock, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            std::cerr << "[Client] Connection closed or error receiving.\n";
            break;
        }

        std::string response(buffer, bytes_received);
        std::cout << "[Client] Received: " << response << "\n";

        if (!packet.verifyChecksum(response)) {
            std::cerr << "[Client] Invalid checksum in request.\n";
            continue;
        }

        std::string flags = extractFlags(response);

        if (std::bitset<9>(flags).to_ulong() == FLAG_DATA_REQUEST){  // FLAG_REQUEST_SENSOR
            std::cout << "[Client] Received DATA REQUEST from server.\n";

            moistureSensor.generateReading();
            std::string payload = moistureSensor.getBinaryPayload();
            std::string data_packet = packet.DataPacket(seq_num++, 0, payload, FLAG_SENSOR);
            packet.SendPacket(sock, data_packet);
            std::cout << "[Client] Sent sensor data.\n";

            // Wait for ACK from server with retry logic
            bool acked = false;
            for (int retry = 0; retry < 3 && !acked; ++retry) {
                fd_set fds;
                FD_ZERO(&fds);
                FD_SET(sock, &fds);
                struct timeval timeout = {2, 0};

                if (select(sock + 1, &fds, nullptr, nullptr, &timeout) > 0) {
                    memset(buffer, 0, sizeof(buffer));
                    int ack_bytes = recv(sock, buffer, sizeof(buffer), 0);
                    if (ack_bytes > 0) {
                        std::string ack_packet(buffer, ack_bytes);
                        std::string ack_flags = extractFlags(ack_packet);
                        if (packet.verifyChecksum(ack_packet) && std::bitset<9>(ack_flags).to_ulong() == FLAG_ACK) {
                            std::cout << "[Client] ✅ ACK received from server.\n";
                            acked = true;
                        }
                    }
                }
                if (!acked) {
                    std::cout << "[Client] ❗ No ACK received, retrying (" << retry + 1 << "/3)...\n";
                    packet.SendPacket(sock, data_packet);
                }
            }

        } else if (std::bitset<9>(flags).to_ulong() == FLAG_FIN) {
            std::cout << "[Client] Received FIN. Initiating teardown.\n";
            std::string fin_ack = packet.AckPacket(0);
            packet.SendPacket(sock, fin_ack);

            std::string client_fin = packet.FinPacket();
            packet.SendPacket(sock, client_fin);

            memset(buffer, 0, sizeof(buffer));
            bytes_received = recv(sock, buffer, sizeof(buffer), 0);
            if (bytes_received > 0) {
                std::string final_ack(buffer, bytes_received);
                std::cout << "[Client] Received final ACK: " << final_ack << "\n";
            }
            std::cout << "[Client] Connection closed after teardown.\n";
            break;
        }
    }

    client.closeConnection();
    return 0;
}
