# Computer Networks Group 1 - Moon Rover Project 2025

### TCP and UDP Connected Modules

| Connection                      | Protocol | Info                                                     |
|:--------------------------------|:---------|:---------------------------------------------------------|
| Home Server <---> Rover Central | TCP      | Commands must be on a reliable connection                |
| Rover Central <---> Actuators   | TCP      | Steering & movement need to be stable                    |
| Rover Central <---> Sensors     | UDP      | Sensors broadcast constantly and wait for a listener     |
| Inter Rover                     | UDP      | Rover broadcasts constantly and listens for other rovers |

### Home Server to Rover Central Connection

Primary connection with the most latency and likely packet loss. This connection will need to be the main focus in terms of robustness because of the vital nature of the information being exchanged. TCP will result in longer communication times but will also guarantee that the information is received.

### Rover Central Computer to Actuators

**Todo**

### Rover Central Computer to Sensors

**Todo**

### Inter Rover Communications

**Todo**