#pragma once
// Logica pura da FSM de seguranca/controle de carga (sem Arduino -> testavel em host).
// Referencia: .specs/features/controle-termico/spec.md (FR-SAF-*, FR-CTL-*, FR-ALM-*)
#include <cstdint>

namespace thermocore {

enum class ThermalState : uint8_t {
    BOOT,
    SCAN_ONEWIRE,
    SAFE_STOP,     // sensor ausente no boot -> carga bloqueada (CA-SNS-001)
    MONITORING,    // sensor ok, carga off
    HEATER_ON,     // carga ligada (manual)
    ALARM_TEMP,    // temp >= 80 C -> PWM=0 (FR-SAF-001)
    FAIL_SENSOR    // falha de comunicacao -> PWM=0 (FR-SAF-002)
};

enum class BuzzerMode : uint8_t {
    NONE,
    ALARM_TEMP,    // 150ms ON / 2000ms OFF (FR-ALM-001)
    FAIL_SENSOR    // 300ms ON / 5000ms OFF (FR-ALM-002, decisao D-002)
};

enum class AlarmKind : uint8_t {
    NONE,
    TEMP_HIGH,     // alerta termico
    SENSOR_FAIL    // alerta de falha de sensor
};

struct FsmOutput {
    ThermalState state = ThermalState::BOOT;
    int pwm = 0;                 // valor efetivo da carga (0..1023)
    bool loadRequested = false;  // intencao manual do operador
    BuzzerMode buzzer = BuzzerMode::NONE;
    AlarmKind alarm = AlarmKind::NONE;
    bool sensorPresent = false;
};

class ThermalFsm {
public:
    static constexpr float kTempAlert = 80.0f;   // FR-SAF-001
    static constexpr int kPwmMax = 1023;         // 10 bits (decisao D-001)

    // Transicoes de boot
    void begin();              // -> SCAN_ONEWIRE
    void onSensorFound();      // scan encontrou o DS18B20 (FR-SNS-001)
    void onSensorMissing();    // scan sem sensor -> SAFE_STOP

    // Amostragem (1s): valid=false = falha de comunicacao
    void onSample(bool valid, float tempC);

    // Comandos manuais; retornam true se aceitos (intertravamento)
    bool requestOn();
    bool requestOff();

    const FsmOutput& output() const { return out_; }
    const char* stateName() const;

private:
    void updateLoadPwm();

    FsmOutput out_{};
    float temp_ = -127.0f;
    bool tempValid_ = false;
};

} // namespace thermocore
