#include "../socket_server.hpp"
#include "packet.hpp"
#include <iostream>

int main() {
    // Start the server and accept a client (Listening on port 8080)
    SocketServer server(8080);
    if (!server.startListening()) return 1;
    if (!server.acceptClient()) return 1;

    // Prepare the server to receive data
    uint8_t buffer[1024];
    // Receive data from the client
    int bytes = recv(server.getClientSocket(), buffer, sizeof(buffer), 0);

    // Convert the buffer into a vector of bytes
    std::vector<uint8_t> raw(buffer, buffer + bytes);
    // Decode the raw bytes into a Packet
    Packet p = decodePacket(raw);

    // Print the received packet if it is a SENSOR packet and has a payload of size 4
    if (p.type == SENSOR && p.payload.size() == 4) {
        // Interpret the payload as a 4-byte integer
        int value = (p.payload[0] << 24) | (p.payload[1] << 16) |
                    (p.payload[2] << 8)  |  p.payload[3];
        std::cout << "Received SENSOR ID " << (int)p.id << " with value: " << value << "\n";
    }

    return 0;
}
