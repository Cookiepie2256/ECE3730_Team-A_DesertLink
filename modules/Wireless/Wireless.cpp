// Wireless.cpp
// ECE 3730 SAR LoRa Wearables - Wireless Communication Module (Tanner)

#include "Wireless.h"

Wireless*     Wireless::_instance = nullptr;
RadioEvents_t Wireless::_events;

// ---------------------------------------------------------------- setup

bool Wireless::begin(const WirelessConfig& cfg) {
  _cfg = cfg;
  _instance = this;

  // Powers the SX1262, sets up SPI, antenna switch, TCXO and the IRQ task
  // for the RAK4631. Returns 0 on success.
  if (lora_rak4630_init() != 0) {
    return false;
  }

  _events.TxDone    = onTxDone;
  _events.TxTimeout = onTxTimeout;
  _events.RxDone    = onRxDone;
  _events.RxTimeout = onRxTimeout;
  _events.RxError   = onRxError;
  _events.CadDone   = nullptr;

  Radio.Init(&_events);
  Radio.SetChannel(_cfg.frequencyHz);

  Radio.SetTxConfig(MODEM_LORA, _cfg.txPowerDbm, 0, _cfg.bandwidth,
                    _cfg.spreadingFactor, _cfg.codingRate, _cfg.preambleLen,
                    false,  // variable length payload
                    true,   // CRC on
                    0, 0,
                    false,  // IQ not inverted
                    _cfg.txTimeoutMs);

  Radio.SetRxConfig(MODEM_LORA, _cfg.bandwidth, _cfg.spreadingFactor,
                    _cfg.codingRate, 0, _cfg.preambleLen,
                    0,      // symbol timeout
                    false,  // variable length payload
                    0,
                    true,   // CRC on
                    0, 0,
                    false,  // IQ not inverted
                    true);  // continuous RX

  Radio.Standby();
  _listening = false;
  _ready = true;
  return true;
}

// ---------------------------------------------------------------- transmit

Wireless::Result Wireless::send(const uint8_t* data, size_t len) {
  if (!_ready) return Result::NOT_INITIALIZED;
  if (data == nullptr || len == 0 || len > MAX_PAYLOAD) return Result::BAD_LENGTH;

  bool resumeListening = _listening;
  _listening = false;  // radio is half duplex; it cannot hear while sending

  memcpy(_txBuf, data, len);
  _txDone = false;
  _txTimeout = false;

  Radio.Standby();
  Radio.Send(_txBuf, (uint8_t)len);

  // Wait for the TxDone / TxTimeout callback, with our own safety timeout
  uint32_t limit = _cfg.txTimeoutMs + timeOnAirMs(len) + 100;
  uint32_t start = millis();
  while (!_txDone && !_txTimeout && (millis() - start) < limit) {
    delay(1);  // yields to the radio IRQ task
  }

  Result r;
  if (_txDone) {
    _stats.txOk++;
    r = Result::OK;
  } else {
    _stats.txFail++;
    Radio.Standby();
    r = Result::TX_TIMEOUT;
  }

  if (resumeListening) startListening();
  return r;
}

// ---------------------------------------------------------------- receive

void Wireless::startListening() {
  if (!_ready) return;
  _listening = true;
  Radio.Rx(0);  // 0 = continuous receive, no timeout
}

bool Wireless::available() const {
  return _rxReady;
}

Wireless::Result Wireless::read(uint8_t* buf, size_t maxLen, size_t& len) {
  len = 0;
  if (!_ready) return Result::NOT_INITIALIZED;
  if (!_rxReady) return Result::RX_TIMEOUT;

  size_t n = _rxLen;
  if (n > maxLen) {
    _rxReady = false;  // discard so the next packet can arrive
    return Result::BUFFER_TOO_SMALL;
  }
  memcpy(buf, _rxBuf, n);
  len = n;
  _rxReady = false;   // release the slot last, after copying
  return Result::OK;
}

Wireless::Result Wireless::receive(uint8_t* buf, size_t maxLen, size_t& len,
                                   uint32_t timeoutMs) {
  len = 0;
  if (!_ready) return Result::NOT_INITIALIZED;
  if (!_listening) startListening();

  uint32_t start = millis();
  while (!_rxReady) {
    if ((millis() - start) >= timeoutMs) return Result::RX_TIMEOUT;
    delay(1);
  }
  return read(buf, maxLen, len);
}

// ---------------------------------------------------------------- power

void Wireless::standby() {
  if (!_ready) return;
  _listening = false;
  Radio.Standby();
}

void Wireless::sleep() {
  if (!_ready) return;
  _listening = false;
  Radio.Sleep();
}

// ---------------------------------------------------------------- helpers

uint32_t Wireless::timeOnAirMs(size_t len) const {
  if (!_ready) return 0;
  return Radio.TimeOnAir(MODEM_LORA, (uint8_t)len);
}

const char* Wireless::resultToString(Result r) {
  switch (r) {
    case Result::OK:               return "OK";
    case Result::NOT_INITIALIZED:  return "NOT_INITIALIZED";
    case Result::BAD_LENGTH:       return "BAD_LENGTH";
    case Result::TX_TIMEOUT:       return "TX_TIMEOUT";
    case Result::RX_TIMEOUT:       return "RX_TIMEOUT";
    case Result::BUFFER_TOO_SMALL: return "BUFFER_TOO_SMALL";
  }
  return "UNKNOWN";
}

// ---------------------------------------------------------------- radio callbacks
// These run in the radio IRQ task, not in loop(). Keep them short:
// set flags, copy bytes, nothing else.

void Wireless::onTxDone() {
  if (_instance) _instance->_txDone = true;
}

void Wireless::onTxTimeout() {
  if (_instance) _instance->_txTimeout = true;
}

void Wireless::onRxDone(uint8_t* payload, uint16_t size, int16_t rssi, int8_t snr) {
  Wireless* w = _instance;
  if (!w) return;

  if (w->_rxReady) {
    w->_stats.rxDropped++;           // previous packet not read yet
  } else if (size <= MAX_PAYLOAD) {
    memcpy(w->_rxBuf, payload, size);
    w->_rxLen = size;
    w->_lastRssi = rssi;
    w->_lastSnr = snr;
    w->_stats.rxOk++;
    w->_rxReady = true;              // publish last, after the copy
  } else {
    w->_stats.rxErrors++;
  }
  // continuous RX mode keeps listening on its own
}

void Wireless::onRxTimeout() {
  if (_instance && _instance->_listening) Radio.Rx(0);
}

void Wireless::onRxError() {
  if (!_instance) return;
  _instance->_stats.rxErrors++;
  if (_instance->_listening) Radio.Rx(0);
}
