# Team Data Packet and Transmission Plan

**Team name:**  Desert Link (Team A)
**Team members:**  (Gage, Andy, Tanner, Malainy, Kevin)
**Team leader:**  Gage
**Last updated:**  11/5/26

## Discussion 1: Packet Format

Agree on the fields each sensor node will send to the base station. For every field, document its purpose, data type, units, and size before and after compression.

> **Important:** Use elapsed time measured by each node (for example, milliseconds since the node started). It is not a shared clock between nodes.

| Field | Purpose | Data type | Units / format | Size before compression (bytes) | Size after compression (bytes) |
|---|---|---|---|---:|---:|
| Node ID | Identifies the monitoring node | int | none | 4 bytes | 1 byte |
| Sequence number | Identifies each packet and helps detect missing packets | float | none | 4 bytes | 2 bytes |
| Elapsed time | Records when the node created the packet, relative to that node’s startup | float | ms | 4 bytes| 4 bytes |
| GPS latitude | Reports the node’s latitude | float | degrees | 4 bytes | 4 bytes |
| GPS longitude | Reports the node’s longitude | float | degrees | 4 bytes | 4 bytes |
| GPS speed | Reports the node’s speed | float | m/s | 4 bytes | 2 bytes |
| Temperature | Reports the measured temperature | float | F | 4 bytes | 1 byte |
| Humidity | Reports the measured relative humidity | float | % RH | 4 bytes | 1 byte |
| Status / validity | Indicates whether sensor readings are valid or available | int | none | 4 bytes | 1 byte |
| Customized field | Describe an additional field, if needed |  |  |  |  |

**Total packet data size before compression:** ___36___ bytes  
**Total packet data size after compression:** ___20___ bytes  

### Data representation and compression notes

Describe how your team will encode each value. For example, will the packet use binary values or readable text? If you scale a value before storing it as an integer, document the scale.

| Field | Encoding / compression method | Example |
|---|---|---|
| Node ID | 1-byte unsigned integer (`uint8_t`) transmitted directly as binary. | `0x01` (Node 1) |
| Sequence number | 2-byte unsigned integer (`uint16_t`) transmitted directly as binary. | `0x00A2` (Sequence 162) |
| Elapsed time | 4-byte unsigned integer (`uint32_t`) representing milliseconds since startup. | `0x000186A0` (100,000 ms) |
| GPS latitude | 4-byte signed integer (`int32_t`) scaled by $10^7$. | `371041000` (37.104100° N) |
| GPS longitude | 4-byte signed integer (`int32_t`) scaled by $10^7$. | `-1135841000` (-113.584100° W) |
| GPS speed | 2-byte unsigned integer (`uint16_t`) scaled by $100$. | `542` (5.42 m/s) |
| Temperature | 1-byte signed integer (`int8_t`) with a +40°F offset. | `112` (72°F) |
| Humidity | 1-byte unsigned integer (`uint8_t`) scaled by $2$. | `90` (45.0% RH) |
| Status / validity | 1-byte bitfield (`uint8_t`) indicating sensor validity flags. | `0x03`  |


**Other information needed to interpret the packet:**

- Packet format or field order: Packet format or field order: Packed contiguous byte array in Little-Endian byte order: Node ID (1b), Sequence number (2b), Elapsed time (4b), GPS latitude (4b), GPS longitude (4b), GPS speed (2b), Temperature (1b), Humidity (1b), Status / validity (1b). Total size: 20 bytes.

- How missing or invalid sensor values are represented: Indicated via the Status / validity bitfield flags and sentinel values (e.g., 0x7FFFFFFF for GPS coordinates, 0xFF for temperature/humidity, and 0xFFFF for speed).

- Other notes: Utilizes a packed C-struct (__attribute__((packed))) on the RAK4631 board to prevent memory padding, minimizing payload size for efficient LoRa transmission.

## Discussion 2: Data Transmission Coordination



### 1. How will the three nodes take turns transmitting?

Describe the method your team chose. Include how each node knows when it may transmit and how the plan reduces collisions.

**Agreed method:**

- Node 1:
- Node 2:
- Node 3:
- Base station role, if applicable:
- Transmission interval or schedule:
- What happens if a node misses its scheduled turn:

### 2. What happens if a packet is missed or delayed?

Describe how the base station will identify a missing packet, handle delayed or duplicate packets, and show the most recent data.

**Agreed method:**

- Detecting a missing packet: The base station tracks the Sequence number independently for each Node ID. A gap in the sequence numbers (e.g., receiving sequence 102 immediately after sequence 100) indicates that packet 101 was missed due to packet loss or interference.

- Handling a delayed packet: If a packet arrives out of order (e.g., packet 101 arrives after packet 102 because of relay routing delays), the base station logs its historical timestamp (Elapsed time) accurately in the database, but does not overwrite the current dashboard view if a newer sequence number has already been displayed.

- Handling a duplicate packet:Because packets may travel through multiple relay paths in a multi-hop network, the base station maintains a short-term cache of recently received (Node ID, Sequence number) pairs. Any incoming packet matching a sequence number already processed for that node is recognized as a duplicate and dropped to prevent redundant logging.

- Displaying or logging stale data: If no packet is received from a specific field node within a defined timeout window (e.g., 3 consecutive reporting intervals), the base station flags that node's status as "Stale" or "Offline" on the UI/logs, displaying the timestamp of the last received packet so operators know the last known position and health.

### 3. Task owner and implementation file

**Owner(s):**  Gage 
**Library or file name:**  getGPS
**What this code will do:**  
The getGPS library interfaces with the GPS module over UART to read and parse NMEA sentences, extracting essential metrics like latitude, longitude, ground speed, and fix status. It provides clean functions for your main program to check for a fresh fix and retrieve the latest coordinates right before packing them into your 20-byte LoRa payload.