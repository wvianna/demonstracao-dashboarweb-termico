// SAEG-2026 - Sistema de Monitoramento e Controle Termico ESP8266
// Loop principal nao-bloqueante (sem delay/ISR p/ amostragem - NFR-CONC-001).
// Referencia: .specs/features/controle-termico/spec.md
#include <Arduino.h>
#include "hal/pins.h"
#include "sensor/temperature.h"
#include "net/wifi_ap.h"
#include "metrics/system_health.h"
#include "web/server.h"
#include "web/history.h"
#include "thermal_fsm.h"
#include "buzzer.h"

using namespace thermocore;

static TemperatureSensor sensor;
static WifiAP wifi;
static SystemHealth health;
static ThermalServer server;
static ThermalFsm fsm;
static Buzzer buzzer;
static TrendBuffer trend;

void setup() {
    Serial.begin(SERIAL_BAUD);
    pinMode(PIN_LOAD, OUTPUT);
    digitalWrite(PIN_LOAD, LOW);   // carga OFF no boot (fail-safe)
    pinMode(PIN_BUZZER, OUTPUT);
    digitalWrite(PIN_BUZZER, LOW);

    fsm.begin();
    buzzer.setMode(BuzzerMode::NONE);

    // T-003: scan OneWire no boot (FR-SNS-001) - one-shot
    if (sensor.begin()) {
        fsm.onSensorFound();
        Serial.println(F("[boot] DS18B20 detectado"));
    } else {
        fsm.onSensorMissing();
        Serial.println(F("[boot] sensor NAO detectado - carga bloqueada"));
    }

    wifi.begin();
    Serial.printf("[boot] AP: %s -> http://192.168.4.1\n", wifi.ssid());

    server.setFsm(&fsm);
    server.setSensor(&sensor);
    server.setHealth(&health);
    server.setTrend(&trend);
    server.begin();

    health.begin();
}

void loop() {
    const uint32_t now = millis();
    const uint32_t loopStartUs = micros(); // base para medir idle (FR-MET-001)

    // Agendador do buzzer (tick 10ms) - FR-ALM-001..003
    static uint32_t lastTick = 0;
    if (now - lastTick >= 10) {
        lastTick = now;
        buzzer.setMode(fsm.output().buzzer);
        buzzer.tick(now);
        digitalWrite(PIN_BUZZER, buzzer.output() ? HIGH : LOW);
    }

    // Amostragem termica a cada 1s (FR-SNS-002) - sem delay/ISR
    static uint32_t lastSample = 0;
    if (now - lastSample >= SAMPLE_INTERVAL_MS) {
        lastSample = now;
        float t = 0.0f;
        const int r = sensor.poll(now, t);
        if (r == 1) {
            fsm.onSample(true, t);
            trend.push(t); // janela de 120 pontos (FR-UI-003 / D-003)
        } else if (r == -1) {
            fsm.onSample(false, 0.0f); // falha -> PWM=0 + alarme (FR-SAF-002)
        }
    }

    // Aplica PWM da FSM a carga (seguranca primeiro: FR-SAF-001/002)
    analogWrite(PIN_LOAD, fsm.output().pwm);

    // Web server responsivo (FR-NET-006)
    server.handle();

    // Metricas: idle (FR-MET-001) + RAM/flash (FR-MET-002)
    health.addBusyUs(micros() - loopStartUs);
    health.loop(millis());
}
