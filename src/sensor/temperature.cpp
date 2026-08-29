#include "temperature.h"
#include "hal/pins.h"
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

static OneWire oneWire(PIN_SENSOR_TEMP);
static DallasTemperature sensors(&oneWire);

bool TemperatureSensor::begin() {
    // O DS18B20 pode demorar um pouco para responder apos alimentacao/contato
    // (diagnostico multi-pino: a passada imediata perdeu, ~0.5s depois respondeu).
    // IMPORTANTE: nesta versao da DallasTemperature, getDeviceCount() apenas le o
    // cache de devices preenchido em begin() -> para re-escancar, chame begin()
    // novamente (re-enumera o barramento). Retry limitado, somente no boot.
    delay(300); // estabiliza o barramento
    failStreak_ = 0;
    sensors.begin();
    present_ = (sensors.getDeviceCount() > 0);
    for (int attempt = 0; !present_ && attempt < 10; attempt++) {
        delay(250);
        sensors.begin(); // re-enumera o barramento (nao usa delay() no loop)
        present_ = (sensors.getDeviceCount() > 0);
    }
    if (present_) {
        sensors.setResolution(12);
        sensors.setWaitForConversion(false); // nao-bloqueante
        requestConversion(0);
    }
    return present_;
}

bool TemperatureSensor::getRom(uint8_t addr[8]) const {
    if (!present_) return false;
    return sensors.getAddress(addr, 0); // re-busca o endereco do dispositivo 0
}

void TemperatureSensor::requestConversion(uint32_t nowMs) {
    if (!present_) return;
    sensors.requestTemperatures();
    conversionRequested_ = true;
    lastRequestMs_ = nowMs;
}

int TemperatureSensor::poll(uint32_t nowMs, float& tempC) {
    if (!present_) return -1; // sensor nunca detectado no boot
    if (!conversionRequested_) {
        requestConversion(nowMs); // primeiro ciclo: agenda e aguarda
        return 0;
    }
    if (nowMs - lastRequestMs_ < kConvMs) return 0; // ainda convertendo

    const float t = sensors.getTempCByIndex(0);
    if (t == DEVICE_DISCONNECTED_C) {
        requestConversion(nowMs); // tenta de novo no proximo ciclo
        // Debounce (D-010): so reporta falha ao FSM apos kFailThreshold leituras
        // consecutivas com erro. Glitch unico/transiente nao desarma a carga.
        failStreak_++;
        // Diagnostico: raiz da falha (contato/timing) so aparece no log serial.
        Serial.printf("[sensor] leitura falhou (streak=%d/%d) t=%lums\n",
                      failStreak_, kFailThreshold, (unsigned long)nowMs);
        if (failStreak_ >= kFailThreshold) return -1; // falha confirmada
        return 0; // transiente: mantem o ultimo estado valido
    }
    if (failStreak_ > 0) {
        Serial.printf("[sensor] leitura recuperada apos %d falha(s) t=%.1fC\n",
                      failStreak_, t);
    }
    tempC = t;
    failStreak_ = 0;
    hasValidTemp_ = true;
    lastValidTemp_ = t;
    requestConversion(nowMs);
    return 1;
}
