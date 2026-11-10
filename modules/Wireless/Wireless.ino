// Wireless.ino
// Test sketch for the Wireless module on two RAK4631 boards.
//
//   Board A: ROLE = ROLE_NODE  -> sends an 18-byte test packet every 5 s,
//                                 then waits for an ACK from the host.
//   Board B: ROLE = ROLE_HOST  -> listens, prints every packet as hex with
//                                 RSSI/SNR, and replies with a 3-byte ACK.
//
// Exercises: begin(), send(), startListening(), available(), read(),
// receive() with timeout, lastRssi(), lastSnr(), timeOnAirMs(), stats(),
// resultToString(). Uses simulated (counting) payload bytes, not sensor data.
// Open Serial Monitor at 115200 on each board.
//
// IMPORTANT: attach the LoRa antenna before powering. Transmitting without
// an antenna can damage the SX1262.

#include "Wireless.h"

#define ROLE_NODE 1
#define ROLE_HOST 2

// ======= change this per board =======
#ifndef ROLE
#define ROLE     ROLE_NODE
#endif
#ifndef NODE_ID
#define NODE_ID  1
#endif
// =====================================

static const size_t   TEST_PKT_LEN   = 18;    // matches planned packet size
static const uint32_t SEND_PERIOD_MS = 5000;
static const uint32_t ACK_TIMEOUT_MS = 1500;
static const uint8_t  ACK_MARKER     = 0xAC;

Wireless radio;

void printHex(const uint8_t* buf, size_t len) {
  for (size_t i = 0; i < len; i++) {
    if (buf[i] < 0x10) Serial.print('0');
    Serial.print(buf[i], HEX);
    Serial.print(' ');
  }
}

void blink(uint8_t pin) {
  digitalWrite(pin, HIGH);
  delay(30);
  digitalWrite(pin, LOW);
}

void setup() {
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  Serial.begin(115200);
  uint32_t t0 = millis();
  while (!Serial && millis() - t0 < 5000) delay(10);

  WirelessConfig cfg;          // defaults: 915 MHz, SF9, 125 kHz, 4/5
  cfg.txPowerDbm = 5;          // bench test at close range; remove for field tests

  if (!radio.begin(cfg)) {
    Serial.println("Radio init FAILED");
    while (true) { blink(LED_GREEN); delay(200); }
  }

  Serial.printf("Radio OK. Role=%s  SF%d  %lu Hz  %d dBm\n",
                ROLE == ROLE_NODE ? "NODE" : "HOST",
                cfg.spreadingFactor, (unsigned long)cfg.frequencyHz, cfg.txPowerDbm);
  Serial.printf("Time on air for %u bytes: %lu ms\n",
                (unsigned)TEST_PKT_LEN, (unsigned long)radio.timeOnAirMs(TEST_PKT_LEN));

  radio.startListening();      // both roles listen when not transmitting
}

#if ROLE == ROLE_NODE
// ------------------------------------------------------------------ NODE
uint8_t seq = 0;
uint32_t lastSend = 0;

void loop() {
  if (millis() - lastSend < SEND_PERIOD_MS) return;
  lastSend = millis();

  // Simulated payload: [node id][seq][counting bytes...]
  uint8_t pkt[TEST_PKT_LEN];
  pkt[0] = NODE_ID;
  pkt[1] = seq;
  for (size_t i = 2; i < TEST_PKT_LEN; i++) pkt[i] = (uint8_t)(i + seq);

  uint32_t t = millis();
  Wireless::Result r = radio.send(pkt, TEST_PKT_LEN);
  Serial.printf("TX seq=%u  %s  (%lu ms)  ", seq,
                Wireless::resultToString(r), (unsigned long)(millis() - t));
  if (r == Wireless::Result::OK) blink(LED_GREEN);

  // Wait for ACK: [0xAC][node id][seq]
  uint8_t ack[8];
  size_t ackLen = 0;
  r = radio.receive(ack, sizeof(ack), ackLen, ACK_TIMEOUT_MS);
  if (r == Wireless::Result::OK && ackLen == 3 &&
      ack[0] == ACK_MARKER && ack[1] == NODE_ID && ack[2] == seq) {
    Serial.printf("ACK  RSSI=%d dBm  SNR=%d dB\n", radio.lastRssi(), radio.lastSnr());
    blink(LED_BLUE);
  } else {
    Serial.printf("no ACK (%s)\n", Wireless::resultToString(r));
  }

  const Wireless::Stats& s = radio.stats();
  if (seq % 10 == 9) {
    Serial.printf("  stats: txOk=%lu txFail=%lu rxOk=%lu rxErr=%lu rxDrop=%lu\n",
                  (unsigned long)s.txOk, (unsigned long)s.txFail, (unsigned long)s.rxOk,
                  (unsigned long)s.rxErrors, (unsigned long)s.rxDropped);
  }
  seq++;
}

#else
// ------------------------------------------------------------------ HOST
void loop() {
  if (!radio.available()) { delay(1); return; }

  uint8_t buf[Wireless::MAX_PAYLOAD];
  size_t len = 0;
  if (radio.read(buf, sizeof(buf), len) != Wireless::Result::OK) return;
  blink(LED_BLUE);

  Serial.printf("RX %u B  RSSI=%d dBm  SNR=%d dB  | ",
                (unsigned)len, radio.lastRssi(), radio.lastSnr());
  printHex(buf, len);
  Serial.println();

  if (len >= 2) {
    delay(20);  // give the node time to switch from TX back to RX
    uint8_t ack[3] = { ACK_MARKER, buf[0], buf[1] };
    Wireless::Result r = radio.send(ack, sizeof(ack));
    Serial.printf("  ACK -> node %u seq %u  %s\n", buf[0], buf[1],
                  Wireless::resultToString(r));
  }
}
#endif
