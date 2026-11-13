#ifndef TIME_SLOT_SCHEDULER_H
#define TIME_SLOT_SCHEDULER_H

#include <Arduino.h>
#include <stdint.h>

class TimeSlotScheduler {
private:
    uint8_t nodeID;              // Assigned Node ID (1, 2, or 3)
    uint32_t slotDurationMs;     // Slot length per node (e.g., 2000 ms)
    uint8_t totalNodes;          // Total nodes in network (3)
    uint32_t cycleDurationMs;    // Full cycle length (slotDurationMs * totalNodes)
    uint64_t lastSentCycle;      // Prevents double transmission within the same slot

public:
    TimeSlotScheduler(uint8_t id, uint32_t slotMs = 2000, uint8_t total = 3);

    // Returns true when the current time hits this node's assigned time slot
    bool isMyTurn(uint64_t currentTimeMs);

    uint32_t getSlotOffsetMs() const;
    uint32_t getCycleDurationMs() const;
};

#endif
