// Wireless.h
// ECE 3730 SAR LoRa Wearables - Wireless Communication Module (Tanner)
//
// Owns the SX1262 radio on the RAK4631. Moves RAW BYTES only.
// It does not know what a packet means; the Data Packet module (Malainy)
// encodes/decodes, and the scheduling/monitor code decides WHEN to send.
//
// Framework: RAK nRF52 Arduino BSP + "SX126x-Arduino" library (beegee-tokyo)
//   Boards Manager URL: https://raw.githubusercontent.com/RAKWireless/RAKwireless-Arduino-BSP-Index/main/package_rakwireless_index.json
//   Board: "WisBlock RAK4631"
//   Library Manager: "SX126x-Arduino"
//
// Test sketch: Wireless.ino (same folder)

#pragma once
#include <Arduino.h>
#include <SX126x-Arduino.h>

// Radio settings. Every board in the system MUST use identical values
// (frequency, SF, BW, CR, preamble) or they will not hear each other.
struct WirelessConfig {
  uint32_t frequencyHz     = 915000000; // US915 band
  int8_t   txPowerDbm      = 22;        // SX1262 max is 22. Use ~5 for bench tests a few feet apart.
  uint8_t  spreadingFactor = 9;         // 7..12. Higher = longer range, longer airtime.
  uint8_t  bandwidth       = 0;         // 0 = 125 kHz, 1 = 250 kHz, 2 = 500 kHz
  uint8_t  codingRate      = 1;         // 1 = 4/5, 2 = 4/6, 3 = 4/7, 4 = 4/8
  uint16_t preambleLen     = 8;         // symbols
  uint32_t txTimeoutMs     = 3000;      // give up on a TX after this long
};

class Wireless {
public:
  static constexpr size_t MAX_PAYLOAD = 64; // our packet is ~18 B, leave headroom

  enum class Result : uint8_t {
    OK,
    NOT_INITIALIZED,
    BAD_LENGTH,
    TX_TIMEOUT,
    RX_TIMEOUT,
    BUFFER_TOO_SMALL,
  };

  struct Stats {
    uint32_t txOk = 0;
    uint32_t txFail = 0;
    uint32_t rxOk = 0;
    uint32_t rxErrors = 0;   // CRC / header errors reported by the radio
    uint32_t rxDropped = 0;  // a new packet arrived before the last one was read
  };

  // ---- Setup ----
  bool begin(const WirelessConfig& cfg = WirelessConfig());

  // ---- Transmit ----
  // Blocking: returns after TX done or timeout. If the radio was listening
  // before the call, it automatically goes back to listening afterwards.
  Result send(const uint8_t* data, size_t len);

  // ---- Receive ----
  void   startListening();                 // continuous RX
  bool   available() const;                // true when a packet is waiting
  Result read(uint8_t* buf, size_t maxLen, size_t& len);          // non-blocking
  Result receive(uint8_t* buf, size_t maxLen, size_t& len,
                 uint32_t timeoutMs);                              // blocking with timeout

  // Link quality of the most recently received packet
  int16_t lastRssi() const { return _lastRssi; }  // dBm
  int8_t  lastSnr()  const { return _lastSnr; }   // dB

  // ---- Power ----
  void standby();  // radio idle, fast to wake
  void sleep();    // lowest power; call startListening() or send() to wake

  // ---- Helpers ----
  uint32_t timeOnAirMs(size_t len) const;  // airtime for a payload of len bytes
  bool isListening() const { return _listening; }
  const Stats& stats() const { return _stats; }
  const WirelessConfig& config() const { return _cfg; }
  static const char* resultToString(Result r);

private:
  // SX126x-Arduino uses plain C callbacks, so these static functions
  // forward into the single Wireless instance.
  static void onTxDone();
  static void onTxTimeout();
  static void onRxDone(uint8_t* payload, uint16_t size, int16_t rssi, int8_t snr);
  static void onRxTimeout();
  static void onRxError();

  static Wireless*     _instance;
  static RadioEvents_t _events;

  WirelessConfig _cfg;
  bool _ready = false;
  bool _listening = false;

  // Written by the radio IRQ task, read by loop()
  volatile bool    _txDone = false;
  volatile bool    _txTimeout = false;
  volatile bool    _rxReady = false;
  volatile size_t  _rxLen = 0;
  volatile int16_t _lastRssi = 0;
  volatile int8_t  _lastSnr = 0;
  uint8_t _rxBuf[MAX_PAYLOAD];
  uint8_t _txBuf[MAX_PAYLOAD];

  Stats _stats;
};
