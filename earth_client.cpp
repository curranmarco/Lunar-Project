#include "Socket/socket_client.hpp" // Custom socket client implementation
#include "Packet/packet_functions.hpp" // Custom packet handling functions

#include <iostream>
#include <string>
#include <bitset>
#include <cstring>
#include <unistd.h>

// Define flag constants for Earth client commands
#define FLAG_EARTH 0x40 // Flag to identify the Earth client
#define FLAG_REQUEST_SENSOR 0x80 // Flag to request sensor data
#define FLAG_REQUEST_ACTUATOR 0x20 // Flag to request actuator speed
#define FLAG_FIN 0x01 // Flag to indicate connection termination

// Function to extract the flags section from a packet
std::string extractFlags(const std::string& packet) {
    size_t flags_start = 16 + 16 + 32 + 32 + 4 + 3; // Position of the flags in the packet
    return packet.substr(flags_start, 9); // Extract 9 bits of flags
}

// Function to send a request to the server and handle the response
void sendRequest(SocketClient& client, Packet& packet, uint8_t flag) {
    std::string payload(32, '0'); // Create an empty payload (32 bits of zeros)
    std::string request = packet.DataPacket(0, 0, payload, flag); // Create a data packet with the specified flag
    packet.SendPacket(client.getSocket(), request); // Send the packet to the server
    std::cout << "[EARTH] Command sent to server. Waiting for response...\n";

    char buffer[2048];
    memset(buffer, 0, sizeof(buffer)); // Clear the buffer

    // Receive the server's response
    int len = recv(client.getSocket(), buffer, sizeof(buffer), 0);
    if (len > 0) {
        std::string response(buffer, len); // Convert the response to a string
        if (packet.verifyChecksum(response)) { // Verify the checksum of the response
            std::cout << "[EARTH] 🌐 Received response from server:\n" << response << "\n";
        } else {
            std::cerr << "[EARTH] ❌ Invalid checksum in response.\n";
        }
    } else {
        std::cerr << "[EARTH] Failed to receive data or connection closed.\n";
    }
}

int main() {
    // Create a socket client and connect to the server
    SocketClient client("127.0.0.1", 8080); // Connect to the server at localhost on port 8080
    if (!client.connectToServer()) return 1; // Exit if the connection fails

    int sock = client.getSocket(); // Get the client socket
    Packet packet(1236, 8080); // Create a packet object with source and destination ports
    char buffer[2048];

    // --- Handshake process ---
    std::string syn = packet.SynPacket(); // Create a SYN packet
    packet.SendPacket(sock, syn); // Send the SYN packet to the server
    std::cout << "[EARTH] Sent SYN.\n";

    memset(buffer, 0, sizeof(buffer)); // Clear the buffer
    int bytes_received = recv(sock, buffer, sizeof(buffer), 0); // Receive the SYN-ACK packet
    std::string synack(buffer, bytes_received);
    std::cout << "[EARTH] Received SYN-ACK.\n";

    if (!packet.verifyChecksum(synack)) { // Verify the checksum of the SYN-ACK packet
        std::cerr << "[EARTH] Invalid SYN-ACK checksum.\n";
        return 1;
    }

    // Extract the server's sequence number from the SYN-ACK packet
    std::string seq_bits = synack.substr(16 + 16, 32);
    uint32_t server_seq = std::bitset<32>(seq_bits).to_ulong();

    // Create and send an ACK packet to complete the handshake
    std::string ack = packet.AckPacket(server_seq + 1);
    packet.SendPacket(sock, ack);
    std::cout << "[EARTH] Sent ACK.\n";

    // --- Identification as EARTH client ---
    std::string identity = packet.DataPacket(0, 0, std::string(32, '0'), FLAG_EARTH); // Create an identity packet
    packet.SendPacket(sock, identity); // Send the identity packet to the server
    std::cout << "[EARTH] Sent identity packet.\n";

    // --- Command loop ---
    while (true) {
        // Display the command menu
        std::cout << "\n🪐 Earth Command Center\n";
        std::cout << "1. Request sensor data\n";
        std::cout << "2. Request actuator speed\n";
        std::cout << "0. Exit\n";
        std::cout << "> ";
        int option;
        std::cin >> option; // Get the user's choice

        if (option == 0) { // Exit the loop if the user chooses 0
            std::cout << "[EARTH] Closing connection.\n";
            break;
        } else if (option == 1) { // Request sensor data
            sendRequest(client, packet, FLAG_REQUEST_SENSOR);
        } else if (option == 2) { // Request actuator speed
            sendRequest(client, packet, FLAG_REQUEST_ACTUATOR);
        } else { // Handle invalid options
            std::cout << "Invalid option.\n";
        }
    }

    client.closeConnection(); // Close the connection to the server
    return 0; // Exit the program
}
