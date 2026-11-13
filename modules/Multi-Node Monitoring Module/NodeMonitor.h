#ifndef NODE_MONITOR_H
#define NODE_MONITOR_H

#include <Arduino.h>
#include <stdint.h>

class NodeMonitor {
public:
    static const uint8_t NUM_NODES = 3;

    NodeMonitor(uint64_t timeoutThresholdMs = 5000);

    void begin();
    void updateNode(uint8_t nodeID, uint64_t currentTimeMs);
    void checkTimeouts(uint64_t currentTimeMs);

    void nodeMonitor(uint8_t nodeID);
    void nodeMonitor(const char* nodeName);
    bool isNodeActive(uint8_t nodeID);
    uint8_t getActiveCount();
    void printAllNodes();

private:
    struct NodeInfo {
        uint8_t nodeID;
        char nodeName[16];
        uint64_t lastReceivedTime;
        bool isActive;
    };

    NodeInfo nodes[NUM_NODES];
    uint64_t timeoutMs;

    int findNodeIndexByID(uint8_t nodeID);
    int findNodeIndexByName(const char* name);
};

#endif
    void printAllNodes();
};
//Header Files (.h): Declare class structures, constants, and function prototypes. *for my memory
#endif
