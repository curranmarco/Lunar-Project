#include "../socket_server.hpp"

using namespace std;

int main() {
    SocketServer server(8080);
    
    if (!server.startListening()) return 1;
    if (!server.acceptClient()) return 1;

    return 0;
}