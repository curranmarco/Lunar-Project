#include "Socket/socket_server.hpp" // Custom socket server implementation
#include "Packet/packet_functions.hpp" // Custom packet handling functions

#include <iostream>
#include <bitset>
#include <string>
#include <cstring>
#include <unistd.h>
#include <thread>
#include <chrono>

// Define flag constants for specific commands
#define FLAG_MOVE_STOP 0x08 // Flag for MOVE/STOP commands
#define FLAG_SPEED_REQ 0x20 // Flag for requesting speed

// Function to send a command packet to the client
void sendCommand(int client_socket, uint16_t flag, uint16_t src_port, uint16_t dest_port, const std::string& data) {
    Packet packet(src_port, dest_port); // Create a packet object with source and destination ports
    uint32_t seq = rand(); // Generate a random sequence number
    uint32_t ack = 0; // Acknowledgment number (not used here)
    std::string cmd = packet.DataPacket(seq, ack, data, flag); // Create the command packet

    // Send the packet to the client
    if (packet.SendPacket(client_socket, cmd)) {
        std::cout << "\n📤 Command packet sent (flag: " << std::bitset<9>(flag) << ")\n";
    } else {
        std::cerr << "❌ Failed to send command packet.\n";
    }
}

// Function to listen for a response from the client
void listenForResponse(int client_socket, bool expectSpeed) {
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer)); // Clear the buffer

    // Read data from the client socket
    ssize_t bytes_received = read(client_socket, buffer, sizeof(buffer));
    if (bytes_received <= 0) {
        std::cerr << "❌ No data received.\n";
        return;
    }

    std::string packet(buffer, bytes_received); // Convert received data to a string
    std::cout << "\n📩 Response packet received:\n" << packet << "\n";

    // Extract and temporarily zero out the checksum field for verification
    size_t checksum_pos = 16 + 16 + 32 + 32 + 4 + 3 + 9; // Position of the checksum in the packet
    std::string checksum_bits = packet.substr(checksum_pos, 16); // Extract the checksum bits
    std::string packet_for_checksum = packet;
    packet_for_checksum.replace(checksum_pos, 16, std::string(16, '0')); // Zero out the checksum field

    // Convert checksum bits to integers for comparison
    uint16_t received_checksum = std::bitset<16>(checksum_bits).to_ulong();
    uint16_t calculated_checksum = Packet::computeChecksum(packet_for_checksum);

    // Display checksum verification details
    std::cout << "[VerifyChecksum] Packet Length:  " << packet.length() << " bits\n";
    std::cout << "[VerifyChecksum] Data Bits:      " << packet_for_checksum << "\n";
    std::cout << "[VerifyChecksum] Checksum Bits:  " << checksum_bits << "\n";
    std::cout << "[VerifyChecksum] Received (dec): " << received_checksum << "\n";
    std::cout << "[VerifyChecksum] Calculated:     " << calculated_checksum << "\n";
    std::cout << "[VerifyChecksum] Match?          " << (received_checksum == calculated_checksum ? "YES" : " NO") << "\n";

    // Check if the checksum is valid
    if (received_checksum != calculated_checksum) {
        std::cerr << "❌ Invalid checksum.\n";
        return;
    }

    std::cout << "✅ Checksum valid.\n";

    // Extract the payload from the packet
    size_t payload_start = checksum_pos + 16; // Payload starts after the checksum
    std::string payload = packet.substr(payload_start);
    std::cout << "📦 Payload: " << payload << "\n";

    // If expecting a speed response, extract and display the speed value
    if (expectSpeed && payload.length() >= 32) {
        std::bitset<8> speed_bits(payload.substr(24, 8)); // Extract 8 bits for speed
        int speed_value = static_cast<int>(speed_bits.to_ulong());
        std::cout << "🚀 Speed value extracted: " << speed_value << "\n";
    }
}

int main() {
    srand(static_cast<unsigned>(time(nullptr))); // Seed the random number generator
    int port = 8080; // Server port
    SocketServer server(port); // Create a socket server

    // Start listening for client connections
    if (!server.startListening()) return 1;
    if (!server.acceptClient()) return 1;

    int client_socket = server.getClientSocket(); // Get the client socket

    uint16_t server_port = port; // Server's port
    uint16_t client_port = 5000; // Client's port

    // Main loop to interact with the user and send commands
    while (true) {
        int choice;
        std::cout << "\n🕹️ Choose command to send:\n";
        std::cout << "1. MOVE\n";
        std::cout << "2. STOP\n";
        std::cout << "3. REQUEST SPEED\n";
        std::cout << "0. Exit\n> ";
        std::cin >> choice;

        if (choice == 0) break; // Exit the loop if the user chooses 0

        switch (choice) {
            case 1: { // MOVE command
                std::string move_payload = "00000001" + std::string(24, '0'); // Payload for MOVE
                sendCommand(client_socket, FLAG_MOVE_STOP, server_port, client_port, move_payload);
                listenForResponse(client_socket, false); // Listen for a response
                break;
            }
            case 2: { // STOP command
                std::string stop_payload = "00000000" + std::string(24, '0'); // Payload for STOP
                sendCommand(client_socket, FLAG_MOVE_STOP, server_port, client_port, stop_payload);
                listenForResponse(client_socket, false); // Listen for a response
                break;
            }
            case 3: { // REQUEST SPEED command
                std::string dummy_payload(32, '0'); // Dummy payload for speed request
                sendCommand(client_socket, FLAG_SPEED_REQ, server_port, client_port, dummy_payload);
                listenForResponse(client_socket, true); // Listen for a speed response
                break;
            }
            default: // Handle invalid choices
                std::cout << "Invalid choice.\n";
                continue;
        }

        // Add a short delay before the next interaction
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    return 0; // Exit the program
}