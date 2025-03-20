#include "../socket_client.hpp"

using namespace std;

int main() {
    SocketClient client("134.226.213.137", 8080);

    if (!client.connectToServer()) return 1;

    return 0;
}