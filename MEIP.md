MEIP - Meisaka Encapsulated Interconnect Protocol
=====

##### Draft 1

Design Objectives
-----

- Messages tagged with a channel (word)
- Integrated timestamp features for realtime operation
- both stateless and stateful sessions
- optional message reliability
- optional message ordering
- high level interface for reliable streams
- end of stream mark
- latency and bandwidth tracking
- maximal MTU awareness
- message batching
- priority (3 level) batching / sending
- considerations for encryption of unreliable messages
- Server active / session keepalive messaging
- Server to server and client to server operation modes


Primary Framing layer
-----

```
Packet format
---------------
Len | Purpose |
----|
 2  | Size of all messages in this packet (size of X)
 1  | count of messages in packet
 1  | packet flags
 8  | Media Authentication Code (used by "connected" packets)
 X  | list of messages
```

Message Flags
------

- bit 7:6 - message type (00 = Data, 01 = Control, 10 = ack, 11 = n-ack)
- bit 5 - sequenced
- bit 4 - timestamped

Message Framing layer
------

```
Data message
---------------
Len | Purpose |
----|
 2  | message length (13+X)
 2  | message channel
 1  | message flags
 4  | timestamp (flag)
 4  | message sequence (flag)
 X  | message data

Control message
---------------
Len | Purpose |
----|
 2  | message length
 2  | message channel
 1  | message flags
 4  | message sequence (flag)
 2  | message command
 X  | message data

Ack message
---------------
Len | Purpose |
----|
 2  | message length (5+(4*X))
 2  | message channel
 1  | message flags
 4*X| message sequences acked (flag)

Negative Ack message
---------------
Len | Purpose |
----|
 2  | message length (5+(4*X))
 2  | message channel
 1  | message flags
 4*X| message sequences missing (flag)

```

