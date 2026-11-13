#include "TimeSlotScheduler.h"

TimeSlotScheduler::TimeSlotScheduler(uint8_t id, uint32_t slotMs, uint8_t total)
    : nodeID(id < 1 ? 1 : id),
      slotDurationMs(slotMs == 0 ? 2000 : slotMs),
      totalNodes(total == 0 ? 3 : total),
      lastSentCycle(0xFFFFFFFFFFFFFFFFULL) {
    cycleDurationMs = slotDurationMs * (uint32_t)totalNodes;
}

uint32_t TimeSlotScheduler::getSlotOffsetMs() const {
    // Node 1 -> 0ms, Node 2 -> 2000ms, Node 3 -> 4000ms
    return (uint32_t)(nodeID - 1) * slotDurationMs;
}

uint32_t TimeSlotScheduler::getCycleDurationMs() const {
    return cycleDurationMs;
}

bool TimeSlotScheduler::isMyTurn(uint64_t currentTimeMs) {
    if (nodeID > totalNodes || cycleDurationMs == 0) {
        return false;
    }

    uint64_t currentCycle = currentTimeMs / cycleDurationMs;
    uint32_t timeInCycle = (uint32_t)(currentTimeMs % cycleDurationMs);
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
