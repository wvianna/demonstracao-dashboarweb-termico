#pragma once
// Agendador nao-bloqueante do buzzer (sem Arduino -> testavel em host).
// Referencia: FR-ALM-001..003 | decisao D-002
#include <cstdint>
#include "thermal_fsm.h"  // BuzzerMode compartilhado com a FSM

namespace thermocore {

class Buzzer {
public:
    static constexpr uint32_t kOnTempMs = 150;
    static constexpr uint32_t kOffTempMs = 2000;
    static constexpr uint32_t kOnFailMs = 300;
    static constexpr uint32_t kOffFailMs = 5000;

    void setMode(BuzzerMode mode);
    void tick(uint32_t nowMs);
    bool output() const { return on_; }
    BuzzerMode mode() const { return mode_; }

private:
    BuzzerMode mode_ = BuzzerMode::NONE;
    bool on_ = false;
    uint32_t phaseStartMs_ = 0;
};

} // namespace thermocore
