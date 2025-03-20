# How to use the SocketClient and SocketServer classes

Remember to include the header files in this folder.
Create an object of type Socketserver and specify the port we are going to use as a parameter.

<br />

```
#include "SocketServer.h"

int main() {
    SocketServer server(8080);
    
    if (!server.startListening()) return 1;
    if (!server.acceptClient()) return 1;

    return 0;
}
```

<br />

Use the ```server.startListening()``` method to listen for clients on the provided port. 
The max number of clients that can be queued up is currently 1, but this can be changed if needed.
Note that this method only instructs the device to listen for connection requests and the ```server.acceptClient()``` method is the one that will halt the system and wait until a connection is established. 
If you want to add a delay in between this has to be done separately. 

<br />

For the client, create a SocketClient object and supply the socket and the IP address of the server to connect.
The ```client.connectToServer()``` method begins the attempt to connect to the server with the details specified in the object instantiation.
Similar to ```server.acceptClient()```, this method will wait for a connection or failure once called.

```
#include "SocketClient.h"
int main() {
    SocketClient client("192.168.1.100", 8080);

    if (!client.connectToServer()) return 1;

    return 0;
}
```

<br />

These objects are just for socket connections.
Any actual data transfer will have to be implemented elsewhere.
We will also need to implement port scanning (or hard code the ports we intend to use).