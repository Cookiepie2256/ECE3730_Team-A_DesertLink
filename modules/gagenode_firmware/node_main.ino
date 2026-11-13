#include <Arduino.h>
#include "Wireless.h"
#include "PocketCodec.h"
#include "NodeMonitor.h"

// --- MODULE INSTANCES ---
Wireless wireless;
PocketCodec codec;
NodeMonitor nodeMonitor(10000); // 10 second timeout threshold for node dropouts

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);
  Serial.println("[BASE STATION] Initializing RAK4631 Base Station Receiver...");

  nodeMonitor.begin();

  // Initialize Wireless with configuration identical to nodes
  WirelessConfig config;
  config.frequencyHz = 915000000; // US915 band
  config.txPowerDbm = 14;
  config.spreadingFactor = 9;     // Must match nodes
  config.bandwidth = 0;           // Must match nodes (125 kHz)

  if (!wireless.begin(config)) {
    Serial.println("[ERROR] Wireless initialization failed! Halting.");
    while (true);
  }

  // Start continuous background listening
  wireless.startListening();
  Serial.println("[BASE STATION] Receiver active and listening for Nodes 1, 2, and 3.");
}

void loop() {
  uint64_t currentMillis = millis();

  // Check if a packet is waiting in the wireless buffer
  if (wireless.available()) {
    uint8_t rxBuffer[64];
    size_t rxLen = 0;

    Wireless::Result res = wireless.read(rxBuffer, sizeof(rxBuffer), rxLen);
    if (res == Wireless::Result::OK && rxLen == codec.getPayloadSize()) {
      // Load raw bytes into codec
      codec.loadPayload(rxBuffer);
      uint8_t nodeId = codec.unpackNodeId();

      // Update Node Monitor heartbeat
      nodeMonitor.updateNode(nodeId, currentMillis);

      // Print link quality metrics
      Serial.print("[RX LINK] RSSI: ");
      Serial.print(wireless.lastRssi());
      Serial.print(" dBm | SNR: ");
      Serial.print(wireless.lastSnr());
      Serial.println(" dB");

      // Output JSON telemetry for logging/display
      Serial.println(codec.toJSON());
    } else {
      Serial.print("[WARNING] Packet read error or size mismatch. Result: ");
      Serial.println(Wireless::resultToString(res));
    }
  }

  // Periodically check for node timeouts
  nodeMonitor.checkTimeouts(currentMillis);

  // Small delay to yield CPU
  delay(10);
}