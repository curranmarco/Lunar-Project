#include "../socket_server.hpp"

int main() {
    SocketServer server(8080);

    if (!server.startListening()) {
        std::cerr << "Failed to start the server.\n";
        return -1;
    }

    std::cout << "Server is running and waiting for client connections...\n";
    server.handleConnections();  

    server.closeConnection();
    return 0;
}
