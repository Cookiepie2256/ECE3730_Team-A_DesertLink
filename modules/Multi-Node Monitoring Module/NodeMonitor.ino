#include "NodeMonitor.h"

// 8000 ms (8 second) timeout threshold before a node is marked INACTIVE
NodeMonitor monitor(8000); // Must be greater than cycleDurationMs (6000 ms)

uint64_t lastStatusPrintTime = 0;
const uint64_t STATUS_PRINT_INTERVAL_MS = 5000;

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000) {}

    monitor.begin();

    // TODO: Initialize your wireless transceiver here (e.g., LoRa.begin(), radio.begin(), or SoftwareSerial)

    Serial.println("System ready. Listening for Node_1, Node_2, and Node_3...");
    Serial.println("Bench test: Send '1', '2', or '3' over Serial to trigger a heartbeat.");
}

void loop() {
    uint64_t now = millis();

    // 1. Poll wireless receiver / serial interface for real node heartbeats
    receiveNodeHeartbeats(now);

    // 2. Check if any active node has exceeded the timeout window
    monitor.checkTimeouts(now);

    // 3. Periodically print full status table
    if (now - lastStatusPrintTime >= STATUS_PRINT_INTERVAL_MS) {
        lastStatusPrintTime = now;
        monitor.printAllNodes();
    }
}

void receiveNodeHeartbeats(uint64_t now) {
    // --- OPTION A: Wireless Transceiver Input (LoRa, nRF24L01, HC-12, etc.) ---
    // Replace this block with your transceiver's packet read logic:
    /*
    if (radio.available()) {
        uint8_t incomingNodeID = 0;
        radio.read(&incomingNodeID, sizeof(incomingNodeID));
        monitor.updateNode(incomingNodeID, now);
    }
    */

    // --- OPTION B: Direct UART / Serial Monitor Input (Non-blocking) ---
    // Accepts raw byte IDs (1, 2, 3) from another microcontroller OR ASCII ('1', '2', '3') from Serial Monitor
    while (Serial.available() > 0) {
        int incoming = Serial.read();

        if (incoming == '\r' || incoming == '\n' || incoming == ' ') {
            continue;
        }

        uint8_t nodeID = 0;
        if (incoming >= '1' && incoming <= '9') {
            nodeID = (uint8_t)(incoming - '0'); // ASCII character to numeric ID
        } else {
            nodeID = (uint8_t)incoming;          // Raw byte ID
        }

        monitor.updateNode(nodeID, now);
    }
}
