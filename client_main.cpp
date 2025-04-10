#include "Socket/socket_client.hpp"  // Custom socket client implementation
#include "Packet/packet_functions.hpp"  // Packet creation and verification utilities
#include "Sensors/Sensors.hpp"  // Sensor simulation for data generation

#include <iostream>
#include <unistd.h>
#include <cstring>
#include <bitset>
#include <chrono>
#include <thread>

// Define flag constants for packet identification
#define FLAG_SENSOR 0x04  // Identifies the client as a sensor
#define FLAG_ACK 0x10  // Acknowledgment flag
#define FLAG_FIN 0x01  // Finish flag for connection teardown
#define FLAG_DATA_REQUEST 0x80  // Request flag for data from the server

// Extracts the 9-bit flags section from a received packet
std::string extractFlags(const std::string& packet) {
    size_t flags_start = 16 + 16 + 32 + 32 + 4 + 3;  // Offset to the flags section
    return packet.substr(flags_start, 9);  // Extract 9 bits of flags
}

int main() {
    // Initialize the client with server IP and port
    SocketClient client("127.0.0.1", 8080);
    if (!client.connectToServer()) return 1;  // Exit if connection fails

    int sock = client.getSocket();  // Get the socket descriptor
    Packet packet(1234, 8080);  // Initialize packet with client and server ports
    u_int32_t seq = rand();  // Generate a random initial sequence number
    char buffer[4096] = {0};  // Buffer for receiving data

    // --- Handshake ---
    // Step 1: Send SYN packet to initiate connection
    std::string syn = packet.SynPacket();
    packet.SendPacket(sock, syn);
    std::cout << "[Client] Sent SYN:\n" << syn << "\n\n";

    // Step 2: Receive SYN-ACK from server
    int bytes_received = recv(sock, buffer, sizeof(buffer), 0);
    std::string synack(buffer, bytes_received);
    std::cout << "[Client] Received SYN-ACK:\n" << synack << "\n\n";

    // Verify the checksum of the received SYN-ACK
    if (!packet.verifyChecksum(synack)) {
        std::cerr << "[Client] Invalid SYN-ACK checksum.\n";
        return 1;
    }

    // Step 3: Send ACK to complete the handshake
    std::string seq_bits = synack.substr(16 + 16, 32);  // Extract server's sequence number
    uint32_t server_seq = std::bitset<32>(seq_bits).to_ulong();
    std::string ack = packet.AckPacket(server_seq + 1);
    packet.SendPacket(sock, ack);
    std::cout << "[Client] Sent ACK.\n[Client] Handshake complete.\n\n";

    // --- Identify as SENSOR ---
    // Send a packet to identify the client as a sensor
    std::string init_packet = packet.DataPacket(seq++, 0, std::string(32, '0'), FLAG_SENSOR);
    packet.SendPacket(sock, init_packet);
    std::cout << "[Client] Sent role-identification packet (SENSOR).\n";

    // Initialize a simulated sensor
    Sensor moistureSensor("MoistureSensor", 0, 100);
    uint32_t seq_num = 0;  // Sequence number for sensor data packets

    // --- Main communication loop ---
    while (true) {
        // Receive data from the server
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(sock, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            std::cerr << "[Client] Connection closed or error receiving.\n";
            break;
        }

        std::string response(buffer, bytes_received);
        std::cout << "[Client] Received: " << response << "\n";

        // Verify the checksum of the received packet
        if (!packet.verifyChecksum(response)) {
            std::cerr << "[Client] Invalid checksum in request.\n";
            continue;
        }

        // Extract flags from the received packet
        std::string flags = extractFlags(response);

        // Handle data request from the server
        if (std::bitset<9>(flags).to_ulong() == FLAG_DATA_REQUEST) {
            std::cout << "[Client] Received DATA REQUEST from server.\n";

            // Generate a new sensor reading and send it to the server
            moistureSensor.generateReading();
            std::string payload = moistureSensor.getBinaryPayload();
            std::string data_packet = packet.DataPacket(seq_num++, 0, payload, FLAG_SENSOR);
            packet.SendPacket(sock, data_packet);
            std::cout << "[Client] Sent sensor data.\n";

            // Wait for an acknowledgment (ACK) from the server with retry logic
            bool acked = false;
            for (int retry = 0; retry < 3 && !acked; ++retry) {
                fd_set fds;
                FD_ZERO(&fds);
                FD_SET(sock, &fds);
                struct timeval timeout = {2, 0};  // 2-second timeout

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

        // Handle connection teardown initiated by the server
        } else if (std::bitset<9>(flags).to_ulong() == FLAG_FIN) {
            std::cout << "[Client] Received FIN. Initiating teardown.\n";

            // Send ACK for the FIN packet
            std::string fin_ack = packet.AckPacket(0);
            packet.SendPacket(sock, fin_ack);

            // Send FIN packet to close the connection
            std::string client_fin = packet.FinPacket();
            packet.SendPacket(sock, client_fin);

            // Wait for the final ACK from the server
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

    // Close the client connection
    client.closeConnection();
    return 0;
}
