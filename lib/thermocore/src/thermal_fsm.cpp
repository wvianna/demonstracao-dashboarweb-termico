#include "thermal_fsm.h"

namespace thermocore {

void ThermalFsm::begin() {
    out_.state = ThermalState::SCAN_ONEWIRE;
    out_.pwm = 0;
    out_.loadRequested = false;
    out_.buzzer = BuzzerMode::NONE;
    out_.alarm = AlarmKind::NONE;
    out_.sensorPresent = false;
    tempValid_ = false;
    temp_ = -127.0f;
}

void ThermalFsm::onSensorFound() {
    out_.sensorPresent = true;
    if (out_.state == ThermalState::SCAN_ONEWIRE ||
        out_.state == ThermalState::SAFE_STOP) {
        out_.state = ThermalState::MONITORING;
        out_.alarm = AlarmKind::NONE;
        out_.buzzer = BuzzerMode::NONE;
        out_.loadRequested = false;
    }
    updateLoadPwm();
}

void ThermalFsm::onSensorMissing() {
    out_.sensorPresent = false;
    out_.loadRequested = false;
    out_.pwm = 0;
    if (out_.state == ThermalState::SCAN_ONEWIRE) {
        out_.state = ThermalState::SAFE_STOP; // CA-SNS-001
    }
}

void ThermalFsm::onSample(bool valid, float tempC) {
    tempValid_ = valid;
    temp_ = valid ? tempC : -127.0f;

    if (!valid) {
        // Falha de comunicacao -> desligamento imediato (FR-SAF-002)
        if (out_.state == ThermalState::HEATER_ON ||
            out_.state == ThermalState::MONITORING ||
            out_.state == ThermalState::ALARM_TEMP) {
            out_.loadRequested = false; // fail-safe: requer novo ON (D-005)
            out_.state = ThermalState::FAIL_SENSOR;
            out_.alarm = AlarmKind::SENSOR_FAIL;
            out_.buzzer = BuzzerMode::FAIL_SENSOR;
        }
        out_.pwm = 0;
        return;
    }

    // Leitura valida: recuperacao de falha/safe-stop (FR-SNS-005)
    if (out_.state == ThermalState::FAIL_SENSOR ||
        out_.state == ThermalState::SAFE_STOP) {
        out_.state = ThermalState::MONITORING;
        out_.alarm = AlarmKind::NONE;
        out_.buzzer = BuzzerMode::NONE;
        out_.loadRequested = false; // permanece OFF (CA-SNS-004)
        updateLoadPwm();
        return;
    }

    // Saida do alarme termico (FR-SAF-004: carga permanece OFF)
    if (out_.state == ThermalState::ALARM_TEMP) {
        if (tempC < kTempAlert) {
            out_.state = ThermalState::MONITORING;
            out_.alarm = AlarmKind::NONE;
            out_.buzzer = BuzzerMode::NONE;
            updateLoadPwm();
        }
        return;
    }

    // MONITORING / HEATER_ON
    if (tempC >= kTempAlert) {
        // Desligamento imediato por temp >= 80 C (FR-SAF-001)
        out_.loadRequested = false;
        out_.state = ThermalState::ALARM_TEMP;
        out_.alarm = AlarmKind::TEMP_HIGH;
        out_.buzzer = BuzzerMode::ALARM_TEMP;
        out_.pwm = 0;
        return;
    }
    updateLoadPwm();
}

bool ThermalFsm::requestOn() {
    if (out_.state == ThermalState::MONITORING ||
        out_.state == ThermalState::HEATER_ON) {
        if (tempValid_ && temp_ < kTempAlert) {
            out_.loadRequested = true; // intertravamento ok (FR-CTL-003)
            updateLoadPwm();
            return true;
        }
    }
    return false; // rejeitado (CA-SAF-002)
}

bool ThermalFsm::requestOff() {
    if (out_.state == ThermalState::MONITORING ||
        out_.state == ThermalState::HEATER_ON) {
        out_.loadRequested = false;
        updateLoadPwm();
        return true;
    }
    return false;
}

void ThermalFsm::updateLoadPwm() {
    bool canRun = out_.loadRequested && tempValid_ && temp_ < kTempAlert &&
                  (out_.state == ThermalState::MONITORING ||
                   out_.state == ThermalState::HEATER_ON);
    out_.pwm = canRun ? kPwmMax : 0;
    if (out_.state == ThermalState::MONITORING ||
        out_.state == ThermalState::HEATER_ON) {
        out_.state = out_.loadRequested ? ThermalState::HEATER_ON
                                        : ThermalState::MONITORING;
    }
}

const char* ThermalFsm::stateName() const {
    switch (out_.state) {
        case ThermalState::BOOT: return "BOOT";
        case ThermalState::SCAN_ONEWIRE: return "SCAN_ONEWIRE";
        case ThermalState::SAFE_STOP: return "SAFE_STOP";
        case ThermalState::MONITORING: return "MONITORING";
        case ThermalState::HEATER_ON: return "HEATER_ON";
        case ThermalState::ALARM_TEMP: return "ALARM_TEMP";
        case ThermalState::FAIL_SENSOR: return "FAIL_SENSOR";
    }
    return "UNKNOWN";
}

} // namespace thermocore
