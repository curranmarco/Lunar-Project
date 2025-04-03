#include "../socket_client.hpp"

using namespace std;

int main() {
    SocketClient client("127.0.0.1", 8080);

    if (!client.connectToServer()) return 1;

    client.sendData(client.getServerSocket(), client.SynPacket());
    std::string syn_ack = client.receiveData();
    cout << "SYN-ACK received: " << syn_ack << std::endl;

    std::bitset<32> sn(syn_ack.substr(32, 32));
    client.sendData(client.getServerSocket(), client.AckPacket(sn.to_ullong(), (uint32_t)syn_ack.size()));
    std::cout << "Handshake complete...\n";

    std::string command = client.receiveData();
    std::cout << "Sensor Data request received\n";

    sn = std::bitset<32>(command.substr(32, 32));
    std::bitset<32> data(2346);
    client.sendData(client.getServerSocket(), client.AckPacket(sn.to_ullong(), sn.size()));

    client.sendData(client.getServerSocket(), client.SensorPacket(sn.to_ullong(), (uint32_t)command.size(), data.to_string()));
    std::string data_ack = client.receiveData();
    std::cout << "Received Data Ack\n";

    while (1);
    return 0;
}