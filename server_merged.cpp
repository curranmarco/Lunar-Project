#include "Socket/socket_server.hpp" // Custom socket server implementation
#include "Packet/packet_functions.hpp" // Custom packet handling functions

#include <iostream>
#include <bitset>
#include <string>
#include <thread>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

// Define flag constants for packet identification
#define FLAG_SENSOR 0x04
#define FLAG_ACTUATOR 0x08
#define FLAG_EARTH 0x40
#define FLAG_REQUEST_SENSOR 0x80
#define FLAG_REQUEST_ACTUATOR 0x20
#define FLAG_ACK 0x10
#define FLAG_FIN 0x01

// Extracts the flags section from a packet
std::string extractFlags(const std::string& packet) {
    return packet.substr(16 + 16 + 32 + 32 + 4 + 3, 9); // Extract 9 bits of flags
}

// Global variables for client sockets and readiness states
int actuatorSock = -1;
int sensorSock = -1;
int earthSock = -1;
bool actuatorReady = false;
bool sensorReady = false;
bool earthReady = false;

// Global packet object for creating and verifying packets
Packet global_packet(8080, 5000);

// Function to forward a packet from one socket to another and optionally patch it to include an ACK flag
void forwardAndRelayResponse(int fromSock, int toSock, bool patchToAck = false) {
    char buffer[2048] = {0};
    ssize_t len = recv(fromSock, buffer, sizeof(buffer), 0); // Receive data from the source socket
    if (len > 0) {
        std::string response(buffer, len);
        if (Packet::verifyChecksum(response)) { // Verify the checksum of the received packet
            // Send an ACK packet back to the sender
            std::string ack = global_packet.AckPacket(0);
            global_packet.SendPacket(fromSock, ack);
            std::cout << "✅ Sent ACK back to data sender.\n";

            if (patchToAck) { // If patching is required, modify the flags to include ACK
                std::string patched = response;
                size_t flag_index = 16 + 16 + 32 + 32 + 4 + 3;
                patched.replace(flag_index, 9, std::bitset<9>(FLAG_ACK).to_string());
            
                // Recalculate checksum for the patched packet
                std::string data_no_checksum = patched.substr(0, patched.length() - 16);
                uint16_t new_checksum = global_packet.computeChecksum(data_no_checksum);
                std::string checksum_bits = std::bitset<16>(new_checksum).to_string();
            
                std::string corrected_packet = data_no_checksum + checksum_bits;
                global_packet.SendPacket(toSock, corrected_packet); // Send the patched packet
            } else {
                global_packet.SendPacket(toSock, response); // Forward the original packet
            }
        } else {
            std::cerr << "❌ Invalid checksum in response." << std::endl;
        }
    } else {
        std::cerr << "❌ Failed to receive data." << std::endl;
    }
}

// Function to handle communication with the Earth client
void handleEarth() {
    std::cout << "🌍 Earth interface ready.\n";
    char buffer[2048];

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t len = recv(earthSock, buffer, sizeof(buffer), 0); // Receive data from Earth
        if (len <= 0) break;

        std::string command(buffer, len);
        if (!Packet::verifyChecksum(command)) continue; // Ignore packets with invalid checksums

        std::string flags = extractFlags(command);
        uint16_t flag_val = std::bitset<9>(flags).to_ulong();

        // Handle requests based on the flags
        if (flag_val == FLAG_REQUEST_SENSOR && sensorReady) {
            global_packet.SendPacket(sensorSock, command); // Forward to sensor
            forwardAndRelayResponse(sensorSock, earthSock, true); // Relay response back to Earth
        } else if (flag_val == FLAG_REQUEST_ACTUATOR && actuatorReady) {
            global_packet.SendPacket(actuatorSock, command); // Forward to actuator
            forwardAndRelayResponse(actuatorSock, earthSock); // Relay response back to Earth
        } else {
            std::cerr << "⚠️ Unknown or unready client for command." << std::endl;
        }
    }
}

// Function to handle the initial handshake and communication with a client
void handleClient(int sock) {
    char buffer[2048] = {0};

    // Step 1: Receive SYN packet
    recv(sock, buffer, sizeof(buffer), 0);
    std::string syn(buffer);
    std::cout << "📥 Received SYN:\n" << syn << std::endl;
    if (!Packet::verifyChecksum(syn)) return;

    // Step 2: Send SYN-ACK packet
    std::string seq_bits = syn.substr(32, 32);
    uint32_t seq = std::bitset<32>(seq_bits).to_ulong();
    std::string synack = global_packet.SynAckPacket(seq);
    global_packet.SendPacket(sock, synack);
    std::cout << "📤 Sent SYN-ACK" << std::endl;

    // Step 3: Receive ACK packet
    memset(buffer, 0, sizeof(buffer));
    recv(sock, buffer, sizeof(buffer), 0);
    std::string ack(buffer);
    if (!Packet::verifyChecksum(ack)) return;
    std::cout << "✅ Handshake complete" << std::endl;

    // Step 4: Receive identity packet to determine client role
    memset(buffer, 0, sizeof(buffer));
    recv(sock, buffer, sizeof(buffer), 0);
    std::string identity(buffer);
    std::string flags = extractFlags(identity);
    uint16_t role = std::bitset<9>(flags).to_ulong();

    // Assign the client to the appropriate role
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
        std::thread earthThread(handleEarth); // Start a thread to handle Earth communication
        earthThread.detach();
        return;
    }

    // Handle incoming packets from the client
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t len = recv(sock, buffer, sizeof(buffer), 0);
        if (len <= 0) break;
        std::string msg(buffer, len);
        if (Packet::verifyChecksum(msg)) { // Verify checksum of received packets
            std::cout << "[VerifyChecksum] Packet Length:  " << msg.length() << " bits\n";
            std::cout << "[VerifyChecksum] Match?          YES\n";
        }
    }
}

// Main server function to accept and handle client connections
void serverMain() {
    int port = 8080; // Server port
    SocketServer server(port); // Create a socket server
    if (!server.startListening()) return; // Start listening for connections

    std::cout << "🚪 Server is running on port " << port << std::endl;

    while (true) {
        if (!server.acceptClient()) continue; // Accept a new client connection
        int sock = server.getClientSocket();
        std::cout << "🔌 New client connected." << std::endl;
        std::thread clientThread(handleClient, sock); // Start a thread to handle the client
        clientThread.detach();
    }
}

// Entry point of the program
int main() {
    serverMain(); // Start the server
    return 0;
}
