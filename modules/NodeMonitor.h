#ifndef NODE_MONITOR_H
#define NODE_MONITOR_H

#include <iostream>
#include <cstring>
#include <cstdint>

struct NodeInfo {
    uint8_t nodeID;
    char nodeName[20];
    uint64_t lastReceivedTime;
    bool isActive;
    uint32_t lastSequenceNum;
    float latitude;
    float longitude;
    float temperature;
    float humidity;
};

class MultiNodeMonitor {
private:
    static const uint8_t TOTAL_NODES = 3;
    NodeInfo nodes[TOTAL_NODES];
    uint64_t timeoutMs;

    int findNodeIndexByID(uint8_t nodeID);
    int findNodeIndexByName(const char* name);

public:
    MultiNodeMonitor(uint64_t timeoutThresholdMs = 15000);

    void begin();
    void updateNodeData(uint8_t nodeID, float lat, float lon, float temp, float hum, uint32_t seqNum, uint64_t currentTimeMs);
    void checkTimeouts(uint64_t currentTimeMs);

    void nodeMonitor(uint8_t nodeID);
    void nodeMonitor(const char* nodeName);

    bool isNodeActive(uint8_t nodeID);
    void printAllNodes();
};
//Header Files (.h): Declare class structures, constants, and function prototypes. *for my memory
#endif