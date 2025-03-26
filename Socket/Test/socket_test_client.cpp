#include "../socket_client.hpp"

using namespace std;

int main() {
    SocketClient client("127.0.0.1", 8080);

    if (!client.connectToServer()) return 1;

    return 0;
}