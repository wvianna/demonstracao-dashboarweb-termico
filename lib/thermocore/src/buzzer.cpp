#include "buzzer.h"

namespace thermocore {

void Buzzer::setMode(BuzzerMode mode) {
    if (mode == mode_) return;
    mode_ = mode;
    // Ao iniciar um alarme, começa com o pulso ON (ciclo 150ms/2s ou 300ms/5s)
    on_ = (mode != BuzzerMode::NONE);
    phaseStartMs_ = 0; // primeira chamada a tick() ancora a fase
}

void Buzzer::tick(uint32_t nowMs) {
    if (mode_ == BuzzerMode::NONE) {
        on_ = false;
        phaseStartMs_ = 0;
        return;
    }
    const uint32_t onMs = (mode_ == BuzzerMode::ALARM_TEMP) ? kOnTempMs : kOnFailMs;
    const uint32_t offMs = (mode_ == BuzzerMode::ALARM_TEMP) ? kOffTempMs : kOffFailMs;

    if (phaseStartMs_ == 0) phaseStartMs_ = nowMs;
    const uint32_t elapsed = nowMs - phaseStartMs_;

    if (on_) {
        if (elapsed >= onMs) {
            on_ = false;
            phaseStartMs_ = nowMs;
        }
    } else {
        if (elapsed >= offMs) {
            on_ = true;
            phaseStartMs_ = nowMs;
        }
    }
}

} // namespace thermocore
