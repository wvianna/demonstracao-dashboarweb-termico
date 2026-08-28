#pragma once
// Mapa de pinos normativo (docs/descricao.txt sec.2 | .specs/codebase/TARGET.md)
#define PIN_SENSOR_TEMP 4    // D2 - DS18B20 (OneWire) - validado (ROM 28 FF E2 03 B4 16 05 C2)
#define PIN_BUZZER      16   // D0 - IO16 - saida digital (buzzer)
#define PIN_LOAD        5    // D1 - IO05 - saida PWM 10 bits (resistencia)

#define PWM_MAX         1023 // resolucao PWM 10 bits
#define TEMP_ALERT_C    80.0f // threshold de seguranca (FR-SAF-001)

#define SAMPLE_INTERVAL_MS 1500UL // amostragem 1,5s (FR-SNS-002 | D-011)
#define SERIAL_BAUD     115200

#define AP_IP_1 192
#define AP_IP_2 168
#define AP_IP_3 4
#define AP_IP_4 1
