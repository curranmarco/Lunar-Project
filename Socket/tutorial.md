# How to use the SocketClient and SocketServer classes

Remember to include the header files in this folder.
Create an object of type Socketserver and specify the port we are going to use as a parameter.

```
#include "SocketServer.h"

int main() {
    SocketServer server(8080);
    
    if (!server.startListening()) return 1;
    if (!server.acceptClient()) return 1;

    return 0;
}
```

Use the ```server.startListening()``` method to listen for clients on the provided port. 
The max number of clients that can be queued up is currently 1, but this can be changed if needed.
Note that this method only instructs the device to listen for connection requests and the ```server.acceptClient()``` method is the one that will halt the system and wait until a connection is established. If you want to add a delay in between this has to be done separately.