#include "NodeMonitor.h"

MultiNodeMonitor::MultiNodeMonitor(uint64_t timeoutThresholdMs) 
    : timeoutMs(timeoutThresholdMs) {}

void MultiNodeMonitor::begin() {
    const char* defaultNames[TOTAL_NODES] = {"Node_1", "Node_2", "Node_3"};
    
    for (uint8_t i = 0; i < TOTAL_NODES; i++) {
        nodes[i].nodeID = i + 1;
        std::strncpy(nodes[i].nodeName, defaultNames[i], sizeof(nodes[i].nodeName) - 1);
        nodes[i].nodeName[sizeof(nodes[i].nodeName) - 1] = '\0';
        nodes[i].lastReceivedTime = 0;
        nodes[i].isActive = false;
        nodes[i].lastSequenceNum = 0;
        nodes[i].latitude = 0.0f;
        nodes[i].longitude = 0.0f;
        nodes[i].temperature = 0.0f;
        nodes[i].humidity = 0.0f;
    }
}

int MultiNodeMonitor::findNodeIndexByID(uint8_t nodeID) {
    for (uint8_t i = 0; i < TOTAL_NODES; i++) {
        if (nodes[i].nodeID == nodeID) {
            return i;
        }
    }
    return -1;
}

int MultiNodeMonitor::findNodeIndexByName(const char* name) {
    for (uint8_t i = 0; i < TOTAL_NODES; i++) {
        if (std::strcmp(nodes[i].nodeName, name) == 0) {
            return i;
        }
    }
    return -1;
}

void MultiNodeMonitor::updateNodeData(uint8_t nodeID, float lat, float lon, float temp, float hum, uint32_t seqNum, uint64_t currentTimeMs) {
    int idx = findNodeIndexByID(nodeID);
    if (idx != -1) {
        nodes[idx].latitude = lat;
        nodes[idx].longitude = lon;
        nodes[idx].temperature = temp;
        nodes[idx].humidity = hum;
        nodes[idx].lastSequenceNum = seqNum;
        nodes[idx].lastReceivedTime = currentTimeMs;
        nodes[idx].isActive = true;

        std::cout << "[MONITOR] Updated " << nodes[idx].nodeName 
                  << " (ID " << static_cast<int>(nodeID) << ") - Status: ACTIVE\n";
    } else {
        std::cout << "[MONITOR ERROR] Received packet from unknown Node ID: " << static_cast<int>(nodeID) << "\n";
    }
}

void MultiNodeMonitor::checkTimeouts(uint64_t currentTimeMs) {
    for (uint8_t i = 0; i < TOTAL_NODES; i++) {
        if (nodes[i].isActive && (currentTimeMs - nodes[i].lastReceivedTime > timeoutMs)) {
            nodes[i].isActive = false;
            std::cout << "[WARNING] Missing updates from " << nodes[i].nodeName 
                      << "! Marked as INACTIVE.\n";
        }
    }
}

void MultiNodeMonitor::nodeMonitor(uint8_t nodeID) {
    int idx = findNodeIndexByID(nodeID);
    if (idx == -1) {
        std::cout << "Node ID " << static_cast<int>(nodeID) << " not found.\n";
        return;
    }

    std::cout << "Node Name: " << nodes[idx].nodeName
              << " | ID: " << static_cast<int>(nodes[idx].nodeID)
              << " | Active: " << (nodes[idx].isActive ? "YES" : "NO") << "\n";
}

void MultiNodeMonitor::nodeMonitor(const char* nodeName) {
    int idx = findNodeIndexByName(nodeName);
    if (idx == -1) {
        std::cout << "Node with name '" << nodeName << "' not found.\n";
        return;
    }

    std::cout << "Node Name: " << nodes[idx].nodeName
              << " | ID: " << static_cast<int>(nodes[idx].nodeID)
              << " | Active: " << (nodes[idx].isActive ? "YES" : "NO") << "\n";
}

bool MultiNodeMonitor::isNodeActive(uint8_t nodeID) {
    int idx = findNodeIndexByID(nodeID);
    return (idx != -1) ? nodes[idx].isActive : false;
}

void MultiNodeMonitor::printAllNodes() {
    std::cout << "--- Multi-Node Status Summary ---\n";
    for (uint8_t i = 0; i < TOTAL_NODES; i++) {
        std::cout << "ID: " << static_cast<int>(nodes[i].nodeID)
                  << " | Name: " << nodes[i].nodeName
                  << " | Active: " << (nodes[i].isActive ? "YES" : "NO")
                  << " | Lat/Lon: " << nodes[i].latitude << ", " << nodes[i].longitude
                  << " | Last Seq: " << nodes[i].lastSequenceNum << "\n";
    }
    std::cout << "--------------------------------\n";
    //Class Files (NodeMonitor.cpp): Write the actual internal logic for your specific module.
}