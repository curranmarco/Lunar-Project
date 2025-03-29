#include "/Users/marcocurran/Downloads/UNIVERSITY/3D3/Lunar-Project/Socket/socket_client.hpp"
#include "packet.hpp"
#include <iostream>

int main() {
    // Create a client that will connect to the IP and port
    SocketClient client("127.0.0.1", 8080);
    // Exit if failed 
    if (!client.connectToServer()) return 1;
    
    // Create a packet with type SENSOR, ID 0x01, and payload 42
    Packet p;
    p.type = SENSOR;
    p.id = 0x01;
    p.payload = {0x00, 0x00, 0x00, 0x2A};  // 42 as 4-byte int

    // Serialize the packet into raw binary bytes
    std::vector<uint8_t> raw = encodePacket(p);
    // Send the raw bytes to the server
    send(client.getSocket(), raw.data(), raw.size(), 0);

    // Print a message to the console
    std::cout << "Sent packet with ID " << (int)p.id << " and value 42.\n";

    return 0;
}
