# ECE3730_Team A_DesertLink
TrailLink uses LoRa wireless communication to send GPS location, temperature, and humidity from portable field nodes to a central station. 

## Project Overview
A fun team project with a search-and-rescue connection

## Team Members
| Name | GitHub Username | Responsibility |
|------|-----------------|----------------|
| Malainy | malainyl | data packing |
| Gage | gbo012 | gps module |
| Kevin | EliteDarkLord35 | multi-node monitoring |
| Tanner | tanjeffris | wireless communication |
| Andy | Andy-Holt | environmental sensor |

## Individual Contribution: Tanner (Wireless Communication)

**ECE 3730 | Monitoring System Project | Assignment 1**

| Information | Entry |
|---|---|
| **Student** | Tanner Jeffris |
| **Team** | Team A |
| **Module** | LoRa transmission (Wireless Communication module) |
| **Date** | November 9, 2026 |
| **Repository link** | **TODO: paste link to `modules/Wireless/`** |

### 1. Library Use and Interface

`Wireless` owns the SX1262 LoRa radio on the RAK4631 and moves **raw bytes only**. It does not know what the bytes mean (the Data Packet module encodes and decodes them) and it does not decide when to transmit (the transmission coordination and monitoring code does). Every board in the system uses the same radio settings: 915 MHz, SF9, 125 kHz bandwidth, coding rate 4/5, 8-symbol preamble, CRC on.

**Call order:** `begin()` once in `setup()` → `startListening()` → then `send()` whenever there is data, and `available()` + `read()` (or `receive()` with a timeout) to get incoming packets. `send()` returns the radio to listening automatically if it was listening before.

| Public function | Inputs | Returns or output | Purpose and use notes |
|---|---|---|---|
| `bool begin(const WirelessConfig& cfg = WirelessConfig())` | Optional `WirelessConfig`: `frequencyHz` (Hz, default 915000000), `txPowerDbm` (dBm, default 22, max 22), `spreadingFactor` (7 to 12, default 9), `bandwidth` (0 = 125 kHz), `codingRate` (1 = 4/5), `preambleLen` (symbols, default 8), `txTimeoutMs` (ms, default 3000) | `true` if the radio initialized, `false` on hardware failure | Powers up the SX1262 and applies the radio settings. Call once in `setup()` before any other function. All boards must use identical settings. |
| `Result send(const uint8_t* data, size_t len)` | `data`: byte array to transmit; `len`: 1 to 64 bytes | `Result::OK`, `TX_TIMEOUT`, `BAD_LENGTH`, or `NOT_INITIALIZED` | Blocking transmit. Returns after the packet is on the air (about 185 ms for 18 bytes at SF9) or on timeout. Radio is half duplex, so it cannot receive during this call. |
| `void startListening()` | None | None | Puts the radio in continuous receive mode. Call after `begin()` and after `standby()`/`sleep()`. |
| `bool available() const` | None | `true` if a received packet is waiting | Non-blocking check for the host's main loop. |
| `Result read(uint8_t* buf, size_t maxLen, size_t& len)` | `buf`: destination buffer; `maxLen`: buffer size in bytes | `Result::OK` and `len` = bytes copied; `RX_TIMEOUT` if nothing waiting; `BUFFER_TOO_SMALL` | Non-blocking read of the waiting packet. Frees the slot for the next packet. |
| `Result receive(uint8_t* buf, size_t maxLen, size_t& len, uint32_t timeoutMs)` | Same as `read()`, plus `timeoutMs` (ms) | `Result::OK` with `len` set, or `RX_TIMEOUT` | Blocking receive with a time limit. Use for waiting on an ACK or for a neighbor's packet when relaying. Starts listening if needed. |
| `int16_t lastRssi() const` / `int8_t lastSnr() const` | None | RSSI in dBm / SNR in dB of the last received packet | Link quality for the monitor and dashboard. |
| `void standby()` / `void sleep()` | None | None | Radio power saving. `sleep()` is lowest power; wake with `startListening()` or `send()`. |
| `uint32_t timeOnAirMs(size_t len) const` | `len`: payload bytes | Airtime in ms | Used to size transmission time slots. |
| `const Stats& stats() const` | None | Counters: `txOk`, `txFail`, `rxOk`, `rxErrors`, `rxDropped` | Link reliability numbers for testing and the monitor. |
| `static const char* resultToString(Result r)` | A `Result` value | Readable name, e.g. `"TX_TIMEOUT"` | Debug printing. |

#### Example of Use by Another Team Member

Wearable node: the Data Packet module builds the bytes, this module sends them.

```cpp
#include "Wireless.h"

Wireless radio;

void setup() {
  Serial.begin(115200);
  if (!radio.begin()) {            // default config: 915 MHz, SF9, 125 kHz
    Serial.println("Radio init FAILED");
    while (true) {}
  }
  radio.startListening();
}

void loop() {
  // when it is this node's time slot:
  uint8_t pkt[Wireless::MAX_PAYLOAD];
  size_t len = /* Data Packet module fills pkt and returns its length */ 0;

  Wireless::Result r = radio.send(pkt, len);
  if (r != Wireless::Result::OK) {
    Serial.println(Wireless::resultToString(r));
  }
}
```

Host station: receive bytes and hand them to the packet parser and monitor.

```cpp
void loop() {
  if (radio.available()) {
    uint8_t buf[Wireless::MAX_PAYLOAD];
    size_t len;
    if (radio.read(buf, sizeof(buf), len) == Wireless::Result::OK) {
      // pass buf/len to the Data Packet decoder,
      // pass radio.lastRssi() / radio.lastSnr() to the monitor
    }
  }
}
```

**Setup required:** Arduino IDE with the RAK nRF52 board package (Boards Manager URL is in the header comment of `Wireless.h`), board "WisBlock RAK4631", and the "SX126x-Arduino" library from Library Manager. No pin assignments are needed: the SX1262 is inside the RAK4631 and `begin()` configures its SPI, antenna switch, and TCXO. The LoRa antenna must be attached before powering the board.

### 2. Testing and Results

**This test was run as a desktop simulation, not on RAK4631 hardware** (the team's boards were not available before the deadline). The library and test sketch were first compiled successfully in Arduino IDE for the "WisBlock RAK4631" board. For the run, `sim/run_sim.sh` compiles the **unchanged** `Wireless.cpp` and `Wireless.ino` on Linux against two stub headers in `sim/`: a minimal `Arduino.h` and a mock `SX126x-Arduino.h`. The mock radio has the same API and callbacks as the real library, computes airtime with the Semtech LoRa time-on-air formula, enforces half duplex (packets arriving while not in RX are lost), and carries packets between two programs over a UDP socket on localhost. **RSSI and SNR values in the output are generated by the mock, not measured.**

The test sketch runs as two roles. The node sends an 18-byte **simulated** packet (node ID, sequence number, then counting bytes, no sensor data) every 5 seconds and waits up to 1.5 s for an ACK. The host prints each packet as hex with RSSI and SNR, then sends a 3-byte ACK (`0xAC`, node ID, sequence number). This exercises `begin()`, `send()`, `startListening()`, `available()`, `read()`, `receive()` with timeout, `lastRssi()`, `lastSnr()`, `timeOnAirMs()`, `stats()`, and `resultToString()` in both directions.

| Test item | Description |
|---|---|
| **Test setup** | Linux PC, two terminals (one `host`, one `node`) built by `sim/run_sim.sh`. Radio config as on hardware: 915 MHz, SF9, 125 kHz, CR 4/5, 8-symbol preamble, 5 dBm. Simulated packet loss of 15% on the node's transmissions (`SIM_LOSS=0.15`) to test timeout handling. No hardware or pins used. |
| **Test procedure** | 1. Compile `Wireless.ino` in Arduino IDE for WisBlock RAK4631 (passed). 2. `./run_sim.sh build`. 3. Run `./run_sim.sh host` in one terminal and `SIM_LOSS=0.15 ./run_sim.sh node` in the other. 4. Watch both outputs for several send cycles. |
| **Expected result** | Both programs print "Radio OK" and a time on air of about 185 ms for 18 bytes. The host prints "RX 18 B" with the hex bytes for each received sequence number and sends an ACK. The node prints "TX seq=N OK" followed by "ACK" for each delivered packet, and "no ACK (RX_TIMEOUT)" for any dropped packet, then continues normally. |
| **Observed result** | Matched expected. Time on air reported as 185 ms, and each `send()` returned `OK` after about 186 ms. Packets with sequence 0, 1, 2, 4 and 5 were received by the host with correct bytes and acknowledged. Sequence 3 was dropped by the simulated loss: the host shows a gap from 2 to 4 and the node reported `no ACK (RX_TIMEOUT)`, then recovered on the next cycle. |

> **TODO: insert screenshot of the two terminals side by side**
>
> *Caption: Desktop simulation of the Wireless test sketch. Node (left) shows transmitted sequence numbers, ACKs, and one RX_TIMEOUT from simulated packet loss. Host (right) shows received 18-byte packets with simulated RSSI/SNR. Not hardware output.*

### 3. Current Status and Integration Notes

| Item | Notes |
|---|---|
| **Working** | Compiles for WisBlock RAK4631 in Arduino IDE. In simulation: initialization, transmit, receive, ACK round trip, RSSI/SNR reporting path, time on air calculation, TX/RX timeout handling, and automatic return to listening after `send()`. |
| **Known issues** | **Not yet tested on RAK4631 hardware**; only compiled and run in desktop simulation. Real range, RSSI/SNR, and the SX1262 driver setup in `begin()` are unverified. Only one packet is buffered at a time: if a second packet arrives before `read()` is called, it is dropped and counted in `stats().rxDropped`. Radio is half duplex, so a node cannot hear anything while `send()` is running. Assumes the RAK Arduino BSP; if the team switches to RUI3, the function names stay the same but the internals must be rewritten. |
| **Integration notes** | Input: byte array and length from the Data Packet module's encode function (max 64 bytes; current packet is 18 bytes). Output: received bytes and length to the Data Packet module's decode function on the host, plus `lastRssi()`/`lastSnr()` to the Multi-Node Monitor. Timing (time slots, ACK/retry, relay) is decided by the coordination code, not this module. All boards must use the same `WirelessConfig`. Keep `loop()` from blocking for long so received packets are read before the next one arrives. Raise `txPowerDbm` back to 22 for field use. |
| **Next step** | Run `Wireless.ino` on two RAK4631 boards to confirm the simulated results, then test range outdoors at full power, then integrate with the Data Packet module so real GPS and sensor packets are sent, and confirm the time slot length using `timeOnAirMs()`. |
