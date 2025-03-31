#include "Socket/socket_client.hpp"
#include "Packet/packet_functions.hpp"
#include "Sensors/Sensors.hpp"

#include <iostream>
#include <unistd.h>     // sleep
#include <cstring>      // memset

int main() {
    // Setup connection
    SocketClient client("127.0.0.1", 8080);
    if (!client.connectToServer()) return 1;

    int sock = client.getSocket();
    Packet packet(1234, 8080);  // source and destination port (customize as needed)

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

    std::string ack = packet.AckPacket(0); // You could extract server's ISN from SYN-ACK if needed
    packet.SendPacket(sock, ack);
    std::cout << "[Client] Sent ACK:\n" << ack << "\n\n";
    std::cout << "[Client] Handshake complete.\n\n";

    // --- Data Transmission Loop ---
    Sensor moistureSensor("MoistureSensor", 0, 100);
    uint32_t seq_num = 0;
    uint32_t ack_num = 0; // You could update this if server sends seqs back

    while (true) {
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

        // Advance sequence number (simulate TCP-like behavior)
        seq_num++;
        sleep(2);  // Wait before sending next
    }

  // --- Wait for FIN from server ---
memset(buffer, 0, sizeof(buffer));
bytes_received = recv(sock, buffer, sizeof(buffer), 0);
if (bytes_received <= 0) {
    std::cerr << "[Client] Did not receive FIN. Exiting.\n";
    client.closeConnection();
    return 1;
}

std::string fin_packet(buffer, bytes_received);
std::cout << "[Client] Received FIN from server:\n" << fin_packet << "\n";

if (!packet.verifyChecksum(fin_packet)) {
    std::cerr << "[Client] FIN checksum invalid. Exiting.\n";
    client.closeConnection();
    return 1;
}

// --- Send ACK for FIN ---
std::string final_ack = packet.AckPacket(0); // you can add seq/ack nums if needed
packet.SendPacket(sock, final_ack);
std::cout << "[Client] Sent final ACK:\n" << final_ack << "\n";

// --- Close connection gracefully ---
client.closeConnection();
std::cout << "[Client] Connection closed after FIN.\n";
}

