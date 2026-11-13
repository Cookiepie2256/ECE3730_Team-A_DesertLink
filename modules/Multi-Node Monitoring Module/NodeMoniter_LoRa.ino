#include <SX126x-Arduino.h>
#include "NodeMonitor.h"

// 8000 ms (8 second) timeout threshold before a node is marked INACTIVE
NodeMonitor monitor(8000); // Must be greater than cycleDurationMs (6000 ms)

uint64_t lastStatusPrintTime = 0;
const uint64_t STATUS_PRINT_INTERVAL_MS = 5000;

// --- LoRa Radio Settings (Must match your transmitting nodes) ---
#define RF_FREQUENCY 915000000          // 915 MHz (US/AU), 868 MHz (EU), 923 MHz (AS)
#define LORA_BANDWIDTH 0                // 0: 125 kHz, 1: 250 kHz, 2: 500 kHz
#define LORA_SPREADING_FACTOR 7         // SF7..SF12
#define LORA_CODINGRATE 1               // 1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8
#define LORA_PREAMBLE_LENGTH 8
#define LORA_SYMBOL_TIMEOUT 0
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false

static RadioEvents_t RadioEvents;
volatile bool loraPacketAvailable = false;
volatile uint8_t loraIncomingNodeID = 0;

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);
void OnRxTimeout(void);
void OnRxError(void);

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000) {}

    monitor.begin();

    // Initialize RAK4631 onboard SX1262 LoRa transceiver
    lora_rak4630_init();

    RadioEvents.RxDone = OnRxDone;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.RxError = OnRxError;

    Radio.Init(&RadioEvents);
    Radio.SetChannel(RF_FREQUENCY);
    Radio.SetRxConfig(
        MODEM_LORA,
        LORA_BANDWIDTH,
        LORA_SPREADING_FACTOR,
        LORA_CODINGRATE,
        0,
        LORA_PREAMBLE_LENGTH,
        LORA_SYMBOL_TIMEOUT,
        LORA_FIX_LENGTH_PAYLOAD_ON,
        0,
        true,
        0,
        0,
        LORA_IQ_INVERSION_ON,
        true
    );

    Radio.Rx(0); // Start continuous LoRa reception

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
    // --- OPTION A: Wireless Transceiver Input (RAK4631 SX1262 LoRa) ---
    if (loraPacketAvailable) {
        noInterrupts();
        uint8_t incomingNodeID = loraIncomingNodeID;
        loraPacketAvailable = false;
        interrupts();

        monitor.updateNode(incomingNodeID, now);
    }

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

// --- SX1262 LoRa Interrupt Callbacks ---

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) {
    if (size > 0) {
        uint8_t firstByte = payload[0];
        if (firstByte >= '1' && firstByte <= '9') {
            loraIncomingNodeID = firstByte - '0'; // ASCII character to numeric ID
        } else {
            loraIncomingNodeID = firstByte;       // Raw byte ID
        }
        loraPacketAvailable = true;
    }

    Radio.Rx(0); // Re-enable continuous reception for the next packet
}

void OnRxTimeout(void) {
    Radio.Rx(0);
}

void OnRxError(void) {
    Radio.Rx(0);
}
