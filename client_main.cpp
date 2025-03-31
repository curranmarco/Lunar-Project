#include "Sensors/Sensors.hpp"
#include "Packet/packet_functions.hpp"
#include "Socket/socket_client.hpp"

#include <iostream>
#include <cstdlib>
#include <ctime>

int main() {
    // Seed RNG for consistent randomness across packet and sensor
    srand(static_cast<unsigned>(time(nullptr)));

    // --- CONFIGURATION ---
    std::string server_ip = "127.0.0.1"; // Change to server IP if needed
    int server_port = 8080;              // Server's listening port
    uint16_t source_port = 4000;         // Optional source port
    uint16_t dest_port = 8080;           // Destination port (same as server's)

    // --- 1. Create Sensor ---
    Sensor sensor("LunarMoistureSensor", 0, 100);

    // --- 2. Generate Reading ---
    int reading = sensor.generateReading();
    std::string payload = sensor.getBinaryPayload();

    std::cout << "[Sensor Info]\n";
    std::cout << "Name:     " << sensor.getName() << "\n";
    std::cout << "Location: " << sensor.getLocationName() << " (ID " << sensor.getLocationID() << ")\n";
    std::cout << "Reading:  " << reading << "\n";
    std::cout << "Payload:  " << payload << "\n\n";

    // --- 3. Create Data Packet ---
    Packet packet(source_port, dest_port);
    uint32_t seq_num = rand();     // Random sequence number
    uint32_t ack_num = 0;          // No ACK needed for this first send
    std::string data_packet = packet.DataPacket(seq_num, ack_num, payload);

    // --- 4. Set Up Socket ---
    SocketClient client(server_ip, server_port);
    if (!client.connectToServer(source_port)) {
        std::cerr << " Connection to server failed.\n";
        return 1;
    }

    // --- 5. Send Packet ---
    if (packet.SendPacket(client.getSocket(), data_packet)) {
        std::cout << "Packet sent successfully.\n";
    } else {
        std::cerr << " Failed to send packet.\n";
    }

    return 0;
}
