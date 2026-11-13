#include "NodeMonitor.h"
#include <string.h>

NodeMonitor::NodeMonitor(uint64_t timeoutThresholdMs)
    : timeoutMs(timeoutThresholdMs) {}

void NodeMonitor::begin() {
    const char* defaultNames[NUM_NODES] = {"Node_1", "Node_2", "Node_3"};

    for (uint8_t i = 0; i < NUM_NODES; i++) {
        nodes[i].nodeID = i + 1;
        strncpy(nodes[i].nodeName, defaultNames[i], sizeof(nodes[i].nodeName) - 1);
        nodes[i].nodeName[sizeof(nodes[i].nodeName) - 1] = '\0';
        nodes[i].lastReceivedTime = 0;
        nodes[i].isActive = false;
    }
}

int NodeMonitor::findNodeIndexByID(uint8_t nodeID) {
    for (uint8_t i = 0; i < NUM_NODES; i++) {
        if (nodes[i].nodeID == nodeID) {
            return i;
        }
    }
    return -1;
}

int NodeMonitor::findNodeIndexByName(const char* name) {
    for (uint8_t i = 0; i < NUM_NODES; i++) {
        if (strcmp(nodes[i].nodeName, name) == 0) {
            return i;
        }
    }
    return -1;
}

void NodeMonitor::updateNode(uint8_t nodeID, uint64_t currentTimeMs) {
    int idx = findNodeIndexByID(nodeID);
    if (idx != -1) {
        nodes[idx].lastReceivedTime = currentTimeMs;
        nodes[idx].isActive = true;

        Serial.print("[MONITOR] Heartbeat from ");
        Serial.print(nodes[idx].nodeName);
        Serial.print(" (ID ");
        Serial.print(nodeID);
        Serial.println(") - Status: ACTIVE");
    } else {
        Serial.print("[MONITOR ERROR] Unknown Node ID: ");
        Serial.println(nodeID);
    }
}

void NodeMonitor::checkTimeouts(uint64_t currentTimeMs) {
    for (uint8_t i = 0; i < NUM_NODES; i++) {
        if (nodes[i].isActive && (currentTimeMs - nodes[i].lastReceivedTime > timeoutMs)) {
            nodes[i].isActive = false;
            Serial.print("[WARNING] Timeout on ");
            Serial.print(nodes[i].nodeName);
            Serial.println(" - Marked as INACTIVE.");
        }
    }
}

void NodeMonitor::nodeMonitor(uint8_t nodeID) {
    int idx = findNodeIndexByID(nodeID);
    if (idx == -1) {
        Serial.print("Node ID ");
        Serial.print(nodeID);
        Serial.println(" not found.");
        return;
    }

    Serial.print("Node Name: ");
    Serial.print(nodes[idx].nodeName);
    Serial.print(" | ID: ");
    Serial.print(nodes[idx].nodeID);
    Serial.print(" | Active: ");
    Serial.println(nodes[idx].isActive ? "YES" : "NO");
}

void NodeMonitor::nodeMonitor(const char* nodeName) {
    int idx = findNodeIndexByName(nodeName);
    if (idx == -1) {
        Serial.print("Node with name '");
        Serial.print(nodeName);
        Serial.println("' not found.");
        return;
    }

    Serial.print("Node Name: ");
    Serial.print(nodes[idx].nodeName);
    Serial.print(" | ID: ");
    Serial.print(nodes[idx].nodeID);
    Serial.print(" | Active: ");
    Serial.println(nodes[idx].isActive ? "YES" : "NO");
}

bool NodeMonitor::isNodeActive(uint8_t nodeID) {
    int idx = findNodeIndexByID(nodeID);
    return (idx != -1) ? nodes[idx].isActive : false;
}

uint8_t NodeMonitor::getActiveCount() {
    uint8_t count = 0;
    for (uint8_t i = 0; i < NUM_NODES; i++) {
        if (nodes[i].isActive) {
            count++;
        }
    }
    return count;
}

void NodeMonitor::printAllNodes() {
    Serial.println("--- 3-Node Status Summary ---");
    for (uint8_t i = 0; i < NUM_NODES; i++) {
        Serial.print("ID: ");
        Serial.print(nodes[i].nodeID);
        Serial.print(" | Name: ");
        Serial.print(nodes[i].nodeName);
        Serial.print(" | Active: ");
        Serial.print(nodes[i].isActive ? "YES" : "NO");
        Serial.print(" | Last Seen (ms): ");
        Serial.println((unsigned long)nodes[i].lastReceivedTime);
    }
    Serial.println("-----------------------------");
}
