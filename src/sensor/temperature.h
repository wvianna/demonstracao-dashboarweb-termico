#pragma once
// Driver do DS18B20 (OneWire) - amostragem nao-bloqueante.
// Referencia: FR-SNS-001..005 | NFR-TIM-001 | NFR-CONC-001
#include <stdint.h>

class TemperatureSensor {
public:
    // Scan one-shot no boot (FR-SNS-001). Retorna true se o sensor foi detectado.
    bool begin();

    // Chama a cada iteracao do loop. Retorna:
    //   1  -> leitura valida (tempC preenchido)
    //   0  -> sem dados ainda (conversao em andamento / primeiro ciclo)
    //  -1  -> falha de comunicacao (sensor desconectado)
    int poll(uint32_t nowMs, float& tempC);

    bool present() const { return present_; }

    // ROM (endereco) do dispositivo 0, para diagnostico/log. Retorna false se ausente.
    bool getRom(uint8_t addr[8]) const;

    // Telemetria para o /json (ultima leitura valida)
    bool hasValidTemp() const { return hasValidTemp_; }
    float lastValidTemp() const { return lastValidTemp_; }

private:
    void requestConversion(uint32_t nowMs);

    bool present_ = false;
    bool conversionRequested_ = false;
    uint32_t lastRequestMs_ = 0;
    bool hasValidTemp_ = false;
    float lastValidTemp_ = -127.0f;
    static constexpr uint32_t kConvMs = 800; // 12-bit ~750ms + margem
};
