#include "TimeSlotScheduler.h"

TimeSlotScheduler::TimeSlotScheduler(uint8_t id, uint32_t slotMs, uint32_t total)
    : nodeID(id), slotDurationMs(slotMs), totalNodes(total), lastSentCycle(0xFFFFFFFF) {
    cycleDurationMs = slotDurationMs * totalNodes;
}

uint32_t TimeSlotScheduler::getSlotOffsetMs() const {
    // Calculates slot offset: Node 1 -> 0ms, Node 2 -> 2000ms, Node 3 -> 4000ms
    return (nodeID - 1) * slotDurationMs;
}

bool TimeSlotScheduler::isMyTurn(uint64_t currentTimeMs) {
    uint32_t currentCycle = currentTimeMs / cycleDurationMs;
    uint32_t timeInCycle = currentTimeMs % cycleDurationMs;
    uint32_t slotStart = getSlotOffsetMs();
    uint32_t slotEnd = slotStart + slotDurationMs;

    // Check if current time falls within this node's assigned window
    if (timeInCycle >= slotStart && timeInCycle < slotEnd) {
        if (lastSentCycle != currentCycle) {
            lastSentCycle = currentCycle; // Mark this cycle as sent
            return true;
        }
    }
    return false;
}