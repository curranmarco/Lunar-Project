#include "Socket/socket_client.hpp"
#include "Packet/packet_functions.hpp"
#include "Sensors/Sensors.hpp"

#include <iostream>
#include <unistd.h>
#include <cstring>

int main() {
    SocketClient client("127.0.0.1", 8080);
    if (!client.connectToServer()) return 1;

    int sock = client.getSocket();
    Packet packet(1234, 8080);

    char buffer[4096] = {0};

    // --- Handshake Phase ---
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

    packet.parsePacket(synack);

    std::string ack = packet.AckPacket(0);
    packet.SendPacket(sock, ack);
    std::cout << "[Client] Sent ACK:\n" << ack << "\n\n";
    std::cout << "[Client] Handshake complete.\n\n";

    // --- Data Transmission Loop ---
    Sensor moistureSensor("MoistureSensor", 0, 100);
    uint32_t seq_num = 0;
    uint32_t ack_num = 0;

    for (int i = 0; i < 5; ++i) {
        moistureSensor.generateReading();
        std::string payload = moistureSensor.getBinaryPayload();

        std::string data_packet = packet.DataPacket(seq_num, ack_num, payload);
        packet.SendPacket(sock, data_packet);
        std::cout << "[Client] Sent DATA (seq=" << seq_num << "):\n" << data_packet << "\n\n";

        // --- Wait for ACK ---
        memset(buffer, 0, sizeof(buffer));
        bytes_received = recv(sock, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            std::cerr << "[Client] Failed to receive ACK for DATA. Exiting.\n";
            break;
        }

        std::string ack_response(buffer, bytes_received);
        std::cout << "[Client] Received ACK:\n" << ack_response << "\n";

        if (!packet.verifyChecksum(ack_response)) {
            std::cerr << "[Client] Invalid ACK checksum.\n";
            break;
        }

        packet.parsePacket(ack_response);
        seq_num++;
        sleep(1);
    }

    // --- Wait for FIN from server ---
    memset(buffer, 0, sizeof(buffer));
    bytes_received = recv(sock, buffer, sizeof(buffer), 0);
    if (bytes_received <= 0) {
        std::cerr << "[Client] Did not receive FIN from server. Exiting.\n";
        client.closeConnection();
        return 1;
    }

    std::string server_fin(buffer, bytes_received);
    std::cout << "[Client] Received FIN from server:\n" << server_fin << "\n";

    if (!packet.verifyChecksum(server_fin)) {
        std::cerr << "[Client] FIN checksum invalid. Exiting.\n";
        client.closeConnection();
        return 1;
    }

    packet.parsePacket(server_fin);

    // --- Send ACK for server FIN ---
    std::string fin_ack = packet.AckPacket(0);
    packet.SendPacket(sock, fin_ack);
    std::cout << "[Client] Sent ACK for FIN:\n" << fin_ack << "\n";

    // --- Send client's FIN ---
    std::string client_fin = packet.FinPacket();
    packet.SendPacket(sock, client_fin);
    std::cout << "[Client] Sent FIN:\n" << client_fin << "\n";

    // --- Wait for final ACK from server ---
    memset(buffer, 0, sizeof(buffer));
    bytes_received = recv(sock, buffer, sizeof(buffer), 0);
    if (bytes_received <= 0) {
        std::cerr << "[Client] Did not receive final ACK from server. Exiting.\n";
        client.closeConnection();
        return 1;
    }

    std::string final_ack(buffer, bytes_received);
    std::cout << "[Client] Received final ACK:\n" << final_ack << "\n";

    if (!packet.verifyChecksum(final_ack)) {
        std::cerr << "[Client] Final ACK checksum invalid. Exiting.\n";
        client.closeConnection();
        return 1;
    }

    packet.parsePacket(final_ack);

    client.closeConnection();
    std::cout << "[Client] Connection closed after four-way teardown initiated by server.\n";
}


