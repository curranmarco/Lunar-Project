#include "Packet/packet_functions.hpp" // Custom packet handling functions
#include "Socket/socket_client.hpp" // Custom socket client implementation

#include <iostream>
#include <bitset>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>

// Define flag constants for specific commands
#define FLAG_MOVE_STOP 0x08 // Flag for MOVE/STOP commands
#define FLAG_SPEED_REQ 0x20 // Flag for requesting speed
#define FLAG_ACK 0x10 // Flag for acknowledgment
#define FLAG_FIN 0x01 // Flag for connection termination

// Function to generate a data payload for MOVE/STOP or speed commands
std::string generateDataPayload(bool isMoving, int speed = -1) {
    if (speed != -1) { // If speed is provided, include it in the payload
        std::bitset<8> speed_bits(speed);
        return std::string(24, '0') + speed_bits.to_string(); // 24 bits of padding + 8 bits for speed
    }
    return isMoving ? std::string(32, '1') : std::string(32, '0'); // 32 bits of '1' for MOVE or '0' for STOP
}

// Function to extract the flags section from a packet
std::string extractFlags(const std::string& packet) {
    return packet.substr(16 + 16 + 32 + 32 + 4 + 3, 9); // Extract 9 bits of flags
}

// Function to extract the payload section from a packet
std::string extractPayload(const std::string& packet) {
    size_t start = 16 + 16 + 32 + 32 + 4 + 3 + 9 + 16 + 16 + 16; // Position of the payload in the packet
    return packet.substr(start);
}

int main() {
    srand(time(0)); // Seed the random number generator for sequence numbers and speed values

    // Create a socket client and connect to the server
    SocketClient client("127.0.0.1", 8080); // Connect to the server at localhost on port 8080
    if (!client.connectToServer()) {
        std::cerr << "❌ Could not connect to server.\n";
        return 1; // Exit if the connection fails
    }

    int sock = client.getSocket(); // Get the client socket
    Packet packet(1234, 8080); // Create a packet object with source and destination ports
    uint32_t seq = rand(); // Generate a random initial sequence number
    char buffer[2048] = {0}; // Buffer for receiving data

    // --- Handshake process ---
    std::string syn = packet.SynPacket(); // Create a SYN packet
    packet.SendPacket(sock, syn); // Send the SYN packet to the server

    // Receive the SYN-ACK packet from the server
    ssize_t synack_len = recv(sock, buffer, sizeof(buffer), 0);
    std::string synack(buffer, synack_len);
    if (!Packet::verifyChecksum(synack)) return 1; // Verify the checksum of the SYN-ACK packet

    // Extract the server's sequence number from the SYN-ACK packet
    uint32_t server_seq = std::bitset<32>(synack.substr(32, 32)).to_ulong();

    // Create and send an ACK packet to complete the handshake
    std::string ack = packet.AckPacket(server_seq + 1);
    packet.SendPacket(sock, ack);

    // --- Initialization as actuator client ---
    std::string init = packet.DataPacket(seq++, 0, std::string(32, '0'), FLAG_MOVE_STOP); // Create an initialization packet
    packet.SendPacket(sock, init); // Send the initialization packet to the server
    std::cout << "Actuator client started. Waiting for instructions...\n";

    // --- Main loop to handle incoming instructions ---
    while (true) {
        memset(buffer, 0, sizeof(buffer)); // Clear the buffer
        ssize_t len = recv(sock, buffer, sizeof(buffer), 0); // Receive data from the server
        if (len <= 0) break; // Exit the loop if the connection is closed

        std::string pkt(buffer, len); // Convert the received data to a string
        if (!Packet::verifyChecksum(pkt)) continue; // Ignore packets with invalid checksums

        // Extract flags and payload from the packet
        std::string flags = extractFlags(pkt);
        std::string payload = extractPayload(pkt);
        uint16_t flag_val = std::bitset<9>(flags).to_ulong();

        std::string response; // Variable to store the response payload

        // Handle the received command based on the flags
        if (flag_val == FLAG_MOVE_STOP) { // MOVE/STOP command
            bool move = (payload.substr(0, 8) == "00000001"); // Check if the command is MOVE
            std::cout << (move ? "🚨 Executing: MOVE\n" : "Executing: STOP\n");
            response = generateDataPayload(move); // Generate the response payload
        } else if (flag_val == FLAG_SPEED_REQ) { // Speed request command
            int speed = rand() % 128; // Generate a random speed value (0-127)
            std::cout << "Sending speed: " << speed << "\n";
            response = generateDataPayload(false, speed); // Generate the response payload with the speed value
        } else if (flag_val == FLAG_FIN) { // FIN command (connection termination)
            std::cout << "Received FIN. Closing connection.\n";
            std::string ack_fin = packet.AckPacket(0); // Create an ACK packet for the FIN
            packet.SendPacket(sock, ack_fin); // Send the ACK packet
            break; // Exit the loop
        }

        // Create and send a response packet
        std::string response_pkt = packet.DataPacket(seq++, 0, response, FLAG_ACK);
        packet.SendPacket(sock, response_pkt);

        // Wait for an acknowledgment from the server
        bool acked = false;
        for (int retry = 0; retry < 3 && !acked; retry++) { // Retry up to 3 times if no ACK is received
            fd_set fds;
            FD_ZERO(&fds); FD_SET(sock, &fds); // Set up the file descriptor set
            struct timeval timeout = {2, 0}; // Set a 2-second timeout
            if (select(sock + 1, &fds, nullptr, nullptr, &timeout) > 0) { // Wait for data on the socket
                memset(buffer, 0, sizeof(buffer));
                ssize_t ack_len = recv(sock, buffer, sizeof(buffer), 0); // Receive the ACK packet
                if (ack_len > 0 && Packet::verifyChecksum(std::string(buffer, ack_len))) {
                    std::cout << "ACK received.\n";
                    acked = true; // ACK received successfully
                }
            }
            if (!acked) { // If no ACK is received, resend the response packet
                std::cout << "Resending packet (retry " << retry + 1 << ")\n";
                packet.SendPacket(sock, response_pkt);
            }
        }
    }

    client.closeConnection(); // Close the connection to the server
    return 0; // Exit the program
}
