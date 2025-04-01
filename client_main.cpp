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

///////////////////////PEER CLIENT ///////////////////////////
    /*SocketClient peerClient("127.0.0.1", 9000);
    if (!peerClient.connectToServer()) {
    std::cerr << "[Client] Failed to connect to peer.\n";
    } else {
    std::cout << "[Client] Connected to peer on port 9000.\n";
    }
    int peer_sock = peerClient.getSocket();*/
///////////////////////////////////////////////////////////////////////
    // --- Data Transmission Loop ---
    // --- Data Transmission Loop ---
Sensor moistureSensor("MoistureSensor", 0, 100);
uint32_t seq_num = 0;
uint32_t ack_num = 0;

bool teardown_initiated = false;
while (true) {
    moistureSensor.generateReading();
    std::string payload = moistureSensor.getBinaryPayload();
    int sensorValue = moistureSensor.getLastValue();

    std::string data_packet = packet.DataPacket(seq_num, ack_num, payload);
    packet.SendPacket(sock, data_packet);
    std::cout << "[Client] Sent DATA to Server (seq=" << seq_num << "):\n" << data_packet << "\n\n";
   /* if (sensorValue > 900 && peer_sock > 0) {
        std::string p2p_payload = "Sensor(" + std::to_string(sensorValue) + ")";
        send(peer_sock, p2p_payload.c_str(), p2p_payload.size(), 0);
        std::cout << "[Client] 📤 Sent high value to peer: " << sensorValue << "\n";
    } */
    int retry_count = 0;
    const int max_retries = 5;
    bool ack_received = false;
   

    while (retry_count < max_retries) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);

        struct timeval timeout;
        timeout.tv_sec = 2;
        timeout.tv_usec = 0;

        int activity = select(sock + 1, &readfds, nullptr, nullptr, &timeout);

        if (activity < 0) {
            std::cerr << "[Client] select() error.\n";
            break;
        }

        if (activity > 0 && FD_ISSET(sock, &readfds)) {
            // ACK or FIN received
            memset(buffer, 0, sizeof(buffer));
            int bytes_received = recv(sock, buffer, sizeof(buffer), 0);
            if (bytes_received <= 0) {
                std::cerr << "[Client] Connection closed or error receiving. Exiting.\n";
                teardown_initiated = true;
                break;
            }

            std::string response(buffer, bytes_received);
            std::cout << "[Client] Received Response:\n" << response << "\n";

            if (!packet.verifyChecksum(response)) {
                std::cerr << "[Client] Invalid checksum in response.\n";
                break;
            }

            packet.parsePacket(response);

            if (response.substr(104, 9) == "000000001") {
                std::cout << "[Client] Received FIN from server. Starting teardown...\n";
                teardown_initiated = true;
                break;
            }

            ack_received = true;
            break; // ACK received
        }

        std::cerr << "[Client] No ACK received (attempt " << retry_count + 1 << "). Resending...\n";
        packet.SendPacket(sock, data_packet);
        retry_count++;
    }

    if (!ack_received) {
        std::cerr << "[Client] Failed to receive ACK after " << max_retries << " retries. Giving up.\n";
        break;
    }

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
    if(teardown_initiated){
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
}


