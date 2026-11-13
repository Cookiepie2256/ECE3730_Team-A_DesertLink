// SX126x-Arduino.h (SIMULATION STUB)
// Fake SX1262 radio for desktop testing of the Wireless module.
// "Over the air" is a UDP socket on localhost: the node and the host run
// as two programs in two terminals. Same API and callbacks as the real
// SX126x-Arduino library, so Wireless.cpp is compiled UNCHANGED.
//
// Simulated behavior:
//   - airtime computed with the Semtech LoRa time-on-air formula
//   - half duplex: packets arriving while not in RX mode are lost
//   - RSSI/SNR are generated values (not real measurements)
//   - optional random packet loss: env var SIM_LOSS=0.2 drops 20%
#pragma once
#include "Arduino.h"
#include <cmath>
#include <cstdlib>
#include <random>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#define MODEM_LORA 1

struct RadioEvents_t {
  void (*TxDone)(void);
  void (*TxTimeout)(void);
  void (*RxDone)(uint8_t* payload, uint16_t size, int16_t rssi, int8_t snr);
  void (*RxTimeout)(void);
  void (*RxError)(void);
  void (*CadDone)(bool);
};

#ifndef SIM_MY_PORT
#define SIM_MY_PORT 47001
#endif
#ifndef SIM_PEER_PORT
#define SIM_PEER_PORT 47002
#endif

class SimRadio {
public:
  void Init(RadioEvents_t* ev) { _ev = ev; }
  void SetChannel(uint32_t f) { _freq = f; }
  void SetTxConfig(int, int8_t power, uint32_t, uint32_t bw, uint32_t sf, uint8_t cr,
                   uint16_t preamble, bool, bool, bool, uint8_t, bool, uint32_t) {
    _power = power; _bw = bw; _sf = sf; _cr = cr; _pre = preamble;
  }
  void SetRxConfig(int, uint32_t, uint32_t, uint8_t, uint32_t, uint16_t, uint16_t,
                   bool, uint8_t, bool, bool, uint8_t, bool, bool) {}
  void Standby() { _mode = IDLE; }
  void Sleep()   { _mode = IDLE; }
  void Rx(uint32_t) { _mode = RX; }

  void Send(uint8_t* buf, uint8_t len) {
    memcpy(_txBuf, buf, len);
    _txLen = len;
    _txEnd = millis() + TimeOnAir(MODEM_LORA, len);
    _mode = TX;
  }

  uint32_t TimeOnAir(int, uint8_t len) const {
    // Semtech AN1200.13, explicit header, CRC on
    double bwHz = (_bw == 0) ? 125000.0 : (_bw == 1 ? 250000.0 : 500000.0);
    double tsym = std::pow(2.0, _sf) / bwHz * 1000.0;
    int de = (_sf >= 11 && _bw == 0) ? 1 : 0;
    double num = 8.0 * len - 4.0 * _sf + 28 + 16;
    double nPayload = 8 + std::max(std::ceil(num / (4.0 * (_sf - 2 * de))) * (_cr + 4), 0.0);
    return (uint32_t)std::lround((_pre + 4.25 + nPayload) * tsym);
  }

  void poll() {
    openSocket();
    // finish a transmission once its airtime has elapsed
    if (_mode == TX && (int32_t)(millis() - _txEnd) >= 0) {
      bool drop = (_loss > 0.0) && (std::uniform_real_distribution<>(0, 1)(_rng) < _loss);
      if (!drop) {
        sockaddr_in peer{}; peer.sin_family = AF_INET;
        peer.sin_port = htons(SIM_PEER_PORT);
        peer.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        sendto(_sock, _txBuf, _txLen, 0, (sockaddr*)&peer, sizeof(peer));
      }
      _mode = IDLE;
      if (_ev && _ev->TxDone) _ev->TxDone();
    }
    // deliver anything that arrived "over the air"
    uint8_t buf[256];
    ssize_t n = recv(_sock, buf, sizeof(buf), 0);
    if (n > 0 && _mode == RX && _ev && _ev->RxDone) {
      int16_t rssi = (int16_t)std::lround(std::normal_distribution<>(-48, 2)(_rng));
      int8_t snr = (int8_t)std::lround(std::normal_distribution<>(9, 1)(_rng));
      _ev->RxDone(buf, (uint16_t)n, rssi, snr);
    }
  }

private:
  void openSocket() {
    if (_sock >= 0) return;
    _sock = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in me{}; me.sin_family = AF_INET;
    me.sin_port = htons(SIM_MY_PORT);
    me.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(_sock, (sockaddr*)&me, sizeof(me)) != 0) {
      perror("[SIM] bind failed (is another copy already running?)");
      exit(1);
    }
    fcntl(_sock, F_SETFL, O_NONBLOCK);
    const char* l = getenv("SIM_LOSS");
    _loss = l ? atof(l) : 0.0;
  }

  enum Mode { IDLE, TX, RX } _mode = IDLE;
  RadioEvents_t* _ev = nullptr;
  uint32_t _freq = 0, _bw = 0, _sf = 9, _txEnd = 0;
  uint8_t _cr = 1, _txLen = 0, _txBuf[256];
  uint16_t _pre = 8;
  int8_t _power = 0;
  int _sock = -1;
  double _loss = 0.0;
  std::mt19937 _rng{std::random_device{}()};
};

inline SimRadio Radio;
inline void sim_poll() { Radio.poll(); }

inline uint32_t lora_rak4630_init() {
  printf("[SIMULATION] Mock SX1262 over UDP localhost (port %d -> %d). "
         "RSSI/SNR values are simulated.\n", SIM_MY_PORT, SIM_PEER_PORT);
  return 0;
}
