# Packet Tutorial

This is how the packets we are going to use for this project are composed:

### SYN

The first packet in a TCP handshake, the client is normally the one to send it.
It consists of at least 5 sections:

1) _**Source Port**_
<br>
The port from which the message is sent. 
The client port in this case. 
We can optionally choose to specify the port as a parameter when the connectToServer() method is called or leave it blank to let the OS assign an ephemeral (arbitrary) port. 
This occupies a 16 bit space in the packet.

<br>

```
std::bitset<16> source(ntohs(local_addr.sin_port));
syn += source.to_string() + " ";
```

<br>

2) _**Destination Port**_
<br>
The port to which we want to send the message.
In this case, the server port.
This port will be assigned manually, so we need to work out what ports to use and where. 
The well-known ports range from 0 <-> 1023 and are generally reserved for the system.
Generally, any ports above this range should be safe, but to be absolutely sure, the dynamic ports from 49152 <-> 65535 would be best.
This value should also fit within a 16 bit binary space.

<br>

```
std::bitset<16> dest(ntohs(server_addr.sin_port));
syn += dest.to_string() + " ";
```

<br>

3) _**ISN**_
<br>
The ISN or the initial sequence number is usually used to keep track of where in a sequence a packet belongs. 
The first SN value is a random 32 bit value which can be easily generated.
Each subsequent packet increments the SN.

<br>

```
isn = (rand() % ((int)exp2(32) - 1));
std::bitset<32> isnb(isn);
syn += isnb.to_string() + " ";
```

<br>

4) _**Acknowledgement Number**_
<br>
This field acknowledges that the packets up to the indicated sequence number have been received.
The initial ACK number is set to 0 as no packets have been sent yet.
This value takes up 32 bits of space.

<br>

```
std::bitset<32> ack_num(0);
syn += ack_num.to_string() + " ";
```

<br>

5) _**Data Offset**_
<br>
How many 32 bit words are in the header. 
For our purposes, all headers will have a fixed size of 5 words before data.
This value should always be 5.
This value takes up 4 bits of space.

<br>

5) _**Reserved**_
<br>
3 bits of space which are reserved for future use by convention. 
This value is always 0.

<br>

6) _**Control Flags**_
<br>
Flags used to indicate the packet type.
Syn, Syn-Ack, Ack, Data, Fin, etc.
9 bit value.

7) _**Checksum**_
<br>
Simple enough calculation. Take the binary values of each half word (16 bits) in the header and the data and add them together.
The checksum is the one's complement of the sum, just flip all of the bits.

8) _**Urgent Pointer**_
<br>
We are unlikely to use this field so it is probably safest to just set it to zero.
It is used with the URG flag to indicate a piece of data that needs to be processed urgently but our data is simple enough and it is rarely used in modern TCP packets anyway.
16 bit value.