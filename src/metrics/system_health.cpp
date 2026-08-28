#include "system_health.h"
#include <Arduino.h>

void SystemHealth::begin() {
    lastMs_ = millis();
    busyUs_ = 0;
    idle_ = 0.0f;
}

void SystemHealth::addBusyUs(uint32_t us) {
    busyUs_ += us;
}

void SystemHealth::loop(uint32_t nowMs) {
    if (nowMs - lastMs_ < 1000) return; // janela de 1s (D-009)
    const uint32_t dtMs = nowMs - lastMs_;
    const float busyPct = (float)busyUs_ * 100.0f / (float)(dtMs * 1000UL);
    idle_ = (busyPct > 100.0f) ? 0.0f : 100.0f - busyPct;
    busyUs_ = 0;
    lastMs_ = nowMs;
}

uint32_t SystemHealth::heapFree() const {
    return ESP.getFreeHeap();
}

float SystemHealth::flashUsedPercent() const {
    const uint32_t used = ESP.getSketchSize();
    const uint32_t total = ESP.getFreeSketchSpace() + used;
    return (total == 0) ? 0.0f : (float)used * 100.0f / (float)total;
}
