#include <Arduino.h>
#include "Wireless.h"
#include "GPS.h"
#include "EnvSensor.h"
#include "TimeSlotScheduler.h"
#include "PocketCodec.h"

// --- CONFIGURATION ---
const uint8_t MY_NODE_ID = 1;           // CHANGE TO 2 for Node 2, 3 for Node 3!
const uint32_t SLOT_DURATION_MS = 3000; // 3 seconds per time slot per node

// --- MODULE INSTANCES ---
Wireless wireless;
GPS gps(Serial1, 9600);
EnvSensor envSensor;
TimeSlotScheduler scheduler(MY_NODE_ID, SLOT_DURATION_MS, 3);
PocketCodec codec(MY_NODE_ID);

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);
  Serial.println("[NODE] Initializing RAK4631 Sensor Node...");

  // 1. Initialize GPS Serial (RAK12501 / Quectel L76K)
  if (!gps.begin()) {
    Serial.println("[WARNING] GPS serial initialization failed.");
  }

  // 2. Initialize Environmental Sensor (RAK1901 / SHTC3)
  if (!envSensor.begin()) {
    Serial.println("[WARNING] SHTC3 environment sensor not found on startup. Will retry in loop.");
  }

  // 3. Initialize Wireless (SX1262 on RAK4631)
  WirelessConfig config;
  config.frequencyHz = 915000000; // US915 band
  config.txPowerDbm = 14;         // Adjust power (max 22 dBm; use lower for close bench testing)
  config.spreadingFactor = 9;     // SF9 (must match base station)
  config.bandwidth = 0;           // 125 kHz (must match base station)

  if (!wireless.begin(config)) {
    Serial.println("[ERROR] Wireless initialization failed! Halting.");
    while (true);
  }
  Serial.println("[NODE] Wireless initialized successfully.");
}

void loop() {
  // Continuously ingest incoming GPS serial data
  gps.update();

  // Periodically read environmental sensor
  envSensor.read();

  uint64_t currentMillis = millis();

  // Check TDMA time slot scheduler
  if (scheduler.isMyTurn(currentMillis)) {
    Serial.print("[NODE ");
    Serial.print(MY_NODE_ID);
    Serial.println("] It's my time slot! Gathering sensor data and transmitting...");

    // Fetch GPS Data
    double lat = 0.0, lon = 0.0;
    float alt = 0.0;
    uint8_t sats = 0;
    bool validGPS = gps.getGPS(lat, lon, alt, sats);

    if (!validGPS) {
      Serial.println("[WARNING] GPS fix not valid yet. Sending default/last known coordinates.");
    }

    // Pack data into codec payload
    codec.packLocation(lat, lon, alt);
    codec.packEnvironment(envSensor.getTempF(), envSensor.getHumidity());
    
    // Example system state: 90% battery, status flag 0, uptime in seconds
    codec.packSystemState(90.0, 0, millis() / 1000);

    // Transmit over LoRa using Wireless wrapper
    uint8_t* txBuffer = codec.getEncodedBytes();
    size_t txSize = codec.getPayloadSize();

    Wireless::Result res = wireless.send(txBuffer, txSize);
    if (res == Wireless::Result::OK) {
      Serial.println("[NODE] Packet transmitted successfully!");
    } else {
      Serial.print("[ERROR] Transmission failed with result: ");
      Serial.println(Wireless::resultToString(res));
    }
  }

  // Small delay to yield CPU
  delay(10);
}