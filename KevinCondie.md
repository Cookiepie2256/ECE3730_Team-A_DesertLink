| Information | Entry |
|---|---|
| **Student** | Kevin |
| **Team** | Desert Link |
| **Module** | Multi-Node Monitoring Module (`MultiNodeMonitor`) |
| **Date** | November 8, 2026 |
| **Repository link** | https://github.com/Cookiepie2256/ECE3730_Team-A_DesertLink/blob/main/KevinCondie.md |

## 1. Library Use and Interface

This library provides an object-oriented C++ interface to manage telemetry records for three remote field nodes (Node 1, Node 2, and Node 3) on the RAK4631 base station receiver. It tracks communication health, detects missing updates using a configurable timeout threshold, and provides overloaded functions to query individual node activity status by ID or name.

| Public function | Inputs | Returns or output | Purpose and use notes |
|---|---|---|---|
| `MultiNodeMonitor` | `uint64_t timeoutThresholdMs` (default `15000`) | None (Constructor) | Instantiates the monitor object with a specified timeout limit (in ms) before an un-updated node is flagged as inactive. |
| `begin` | None | `void` | Initializes node tracking array structures with default names (`Node_1`, `Node_2`, `Node_3`) and sets initial states to inactive. Call once in `setup()`. |
| `updateNodeData` | `uint8_t nodeID`, `float lat`, `float lon`, `float temp`, `float hum`, `uint32_t seqNum`, `uint64_t currentTimeMs` | `void` | Ingests parsed payload data for a specific node ID, updates coordinates and sensor data, updates sequence number, and marks node active. |
| `checkTimeouts` | `uint64_t currentTimeMs` | `void` | Evaluates elapsed time since last update for each node; marks node inactive if threshold is exceeded. Call regularly in `loop()`. |
| `nodeMonitor` | `uint8_t nodeID` OR `const char* nodeName` | Serial / Console Output | Overloaded query function. Prints node name, ID, and active status (`YES`/`NO`) to console. |
| `isNodeActive` | `uint8_t nodeID` | `bool` | Returns `true` if the specified node is active, `false` if inactive or not found. |
| `printAllNodes` | None | Serial / Console Output | Prints formatted summary table of all 3 tracked nodes, active statuses, coordinates, and sequence numbers. |

### Example of Use by Another Team Member

To use this library in your own sketch or module, include the header file, instantiate the monitor object, initialize it in `setup()`, and feed parsed telemetry data inside `loop()`:

```cpp
#include <Arduino.h>
#include "NodeMonitor.h"
#include "RAK12501_GPS.h" // Gage's GPS library

// Instantiate Gage's GPS hardware and Kevin's Monitor (10s timeout threshold)
RAK12501_GPS myGPS(Serial1, 9600);
MultiNodeMonitor monitor(10000);

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000);

    // Initialize Gage's GPS and Kevin's Monitor
    myGPS.begin();
    monitor.begin();
}

void loop() {
    double lat = 0.0;
    double lon = 0.0;
    float alt = 0.0;
    uint8_t sats = 0;

    // Retrieve GPS fix data when updated by Gage's module
    if (myGPS.getGPS(lat, lon, alt, sats)) {
        // Ingest telemetry into Node 1 record
        monitor.updateNodeData(1, static_cast<float>(lat), static_cast<float>(lon), 
                              24.5f, 45.0f, 101, millis());
    }

    // Periodically evaluate active statuses and check for timeouts
    monitor.checkTimeouts(millis());

    // Query node health by string name or node ID
    monitor.nodeMonitor("Node_1");
    monitor.nodeMonitor(2);

    delay(1000);
}
```
## 2. Testing and Results

| Test item | Description |
|---|---|
| **Test setup** | RAK4631 WisBlock Core base station connected to a PC via USB for Serial Monitor output at 115200 baud (verified via C++ test harness with a 10,000 ms timeout threshold). Tested using distinct field GPS coordinates, altitudes, and satellite counts assigned to Nodes 1, 2, and 3. |
| **Test procedure** | 1. Initialized the `MultiNodeMonitor` instance using `begin()`.<br>2. Transmitted initial telemetry packets at T = 1,000 ms for Node 1 (37.0975, -113.5670, 850.3m alt), Node 2 (37.1025, -113.5720, 885.5m alt), and Node 3 (37.0945, -113.5650, 820.1m alt).<br>3. Advanced clock to T = 15,000 ms, sending fresh updates for Nodes 1 and 3 while withholding data from Node 2 for 14 seconds.<br>4. Called `checkTimeouts(15000)` to evaluate missing updates.<br>5. Executed `nodeMonitor()` for each node and printed the system table via `printAllNodes()`. |
| **Expected result** | Nodes 1 and 3 should refresh telemetry and report `Active: YES` with sequence numbers 102 and 302. Node 2, having missed updates past the 10,000 ms threshold, should trigger an internal warning and switch status to `Active: NO` while retaining its last sequence number (201). |
| **Observed result** | Nodes 1 and 3 updated successfully and remained `Active: YES`. The monitor printed `[WARNING] Missing updates from Node_2! Marked as INACTIVE.` at T = 15,000 ms, and Node 2 correctly displayed `Active: NO` in both individual `nodeMonitor("Node_2")` queries and the `printAllNodes()` summary table. |
## 3. Current Status and Integration Notes

| Item | Notes |
|---|---|
| **Working** | Multi-node telemetry tracking for 3 nodes, dynamic name/ID status querying via overloaded `nodeMonitor()`, automated timeout/inactivity detection via `checkTimeouts()`, and formatted summary output (`printAllNodes()`). |
| **Known issues** | Have yet to complete integration with Environmental Sensor Module testing but so far GPS testing has functioned well. |
| **Integration notes** | Must call `begin()` in `setup()` prior to handling telemetry data. Call `updateNodeData()` inside the LoRa reception handler when Tanner's wireless module and Malainy's packet parser decode incoming over-the-air packets. Ensure `checkTimeouts(millis())` is called periodically inside `loop()` to keep active/inactive node states up to date. |
| **Next step** | Interface directly with Tanner's SX1262 LoRa module and Malainy's packet module to process live radio payloads across all three deployed field nodes in Assignment 2. |
## Submission Checklist

- [x] Completed report with description of test results
- [x] Library header file (`NodeMonitor.h`) and implementation file (`NodeMonitor.cpp`)
- [x] Test sketch (`main.cpp` / `.ino`) that calls the library’s public functions
- [x] Short demonstration video showing the library working
- [x] All files pushed to the team GitHub repository
