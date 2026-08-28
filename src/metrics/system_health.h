#pragma once
// Metricas de saude do sistema (FR-MET-001..003).
#include <stdint.h>

class SystemHealth {
public:
    void begin();
    void addBusyUs(uint32_t us);   // acumula tempo ocupado do loop (idle)
    void loop(uint32_t nowMs);     // a cada 1s recalcula idle (D-009)

    float idlePercent() const { return idle_; }
    uint32_t heapFree() const;          // RAM disponivel
    float flashUsedPercent() const;     // ocupacao da flash (sketch)

private:
    float idle_ = 0.0f;
    uint32_t lastMs_ = 0;
    uint32_t busyUs_ = 0;
};
