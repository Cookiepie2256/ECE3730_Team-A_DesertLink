/******************************************************************************

                              Online C++ Compiler.
               Code, Compile, Run and Debug C++ program online.
Write your code in this editor and press "Run" button to compile and execute it.

*******************************************************************************/

#include <iostream>
#include "NodeMonitor.h"
#include "TimeSlotScheduler.h"

// Mock function simulating Gage's getGPS interface returning distinct field data per node
bool getGPSForNode(int nodeID, double &lat, double &lon, float &alt, uint8_t &sats) {
    if (nodeID == 1) {
        lat = 37.0975;
        lon = -113.5670;
        alt = 850.3f;
        sats = 9;
        return true;
    } else if (nodeID == 2) {
        lat = 37.1025;
        lon = -113.5720;
        alt = 885.5f;
        sats = 6;
        return true;
    } else if (nodeID == 3) {
        lat = 37.0945;
        lon = -113.5650;
        alt = 820.1f;
        sats = 11;
        return true;
    }
    return false;
}

int main() {
    std::cout << "=========================================================================\n";
    std::cout << " System Test: Time-Slot Transmission Scheduling & Receiver Node Monitor \n";
    std::cout << "=========================================================================\n\n";

    // Receiver: Instantiate Kevin's Multi-Node Monitor (10-second timeout threshold)
    MultiNodeMonitor monitor(10000);
    monitor.begin();

    // Transmitters: Instantiate Time-Slot Schedulers for Nodes 1, 2, and 3 (2000 ms slots)
    TimeSlotScheduler txNode1(1, 2000, 3);
    TimeSlotScheduler txNode2(2, 2000, 3);
    TimeSlotScheduler txNode3(3, 2000, 3);

    uint32_t seqNum1 = 101;
    uint32_t seqNum2 = 201;
    uint32_t seqNum3 = 301;

    double lat = 0.0, lon = 0.0;
    float alt = 0.0;
    uint8_t sats = 0;

    // Run clock loop from T = 0 ms to T = 15,000 ms in steps of 500 ms
    for (uint64_t currentTimeMs = 0; currentTimeMs <= 15000; currentTimeMs += 500) {

        // --- NODE 1 SLOT (Offset: 0 ms) ---
        if (txNode1.isMyTurn(currentTimeMs)) {
            if (getGPSForNode(1, lat, lon, alt, sats)) {
                std::cout << "[T = " << currentTimeMs << " ms] Node 1 transmitting in Slot 1...\n";
                monitor.updateNodeData(1, static_cast<float>(lat), static_cast<float>(lon), 24.5f, 45.0f, seqNum1++, currentTimeMs);
            }
        }

        // --- NODE 2 SLOT (Offset: 2000 ms) ---
        // Simulating signal loss/dropout: Node 2 stops updating after T = 3,000 ms
        if (currentTimeMs <= 3000 && txNode2.isMyTurn(currentTimeMs)) {
            if (getGPSForNode(2, lat, lon, alt, sats)) {
                std::cout << "[T = " << currentTimeMs << " ms] Node 2 transmitting in Slot 2...\n";
                monitor.updateNodeData(2, static_cast<float>(lat), static_cast<float>(lon), 21.0f, 52.3f, seqNum2++, currentTimeMs);
            }
        }

        // --- NODE 3 SLOT (Offset: 4000 ms) ---
        if (txNode3.isMyTurn(currentTimeMs)) {
            if (getGPSForNode(3, lat, lon, alt, sats)) {
                std::cout << "[T = " << currentTimeMs << " ms] Node 3 transmitting in Slot 3...\n";
                monitor.updateNodeData(3, static_cast<float>(lat), static_cast<float>(lon), 19.8f, 60.1f, seqNum3++, currentTimeMs);
            }
        }

        // Check node timeout states
        monitor.checkTimeouts(currentTimeMs);
    }

    // Output final status check and node summary table
    std::cout << "\n--- nodeMonitor() Queries ---\n";
    monitor.nodeMonitor("Node_1");
    monitor.nodeMonitor("Node_2");
    monitor.nodeMonitor("Node_3");
    std::cout << "-----------------------------\n\n";

    monitor.printAllNodes();

    return 0;
}