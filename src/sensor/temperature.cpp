#include "temperature.h"
#include "hal/pins.h"
#include <OneWire.h>
#include <DallasTemperature.h>

static OneWire oneWire(PIN_SENSOR_TEMP);
static DallasTemperature sensors(&oneWire);

bool TemperatureSensor::begin() {
    // O DS18B20 pode demorar um pouco para responder apos alimentacao/contato
    // (diagnostico multi-pino: passada imediata perdeu, ~0.5-1s depois respondeu).
    // Varredura one-shot com retry limitado, apenas no boot (nao no loop).
    delay(300); // estabiliza o barramento
    sensors.begin();
    present_ = (sensors.getDeviceCount() > 0);
    for (int attempt = 0; !present_ && attempt < 10; attempt++) {
        delay(250);
        present_ = (sensors.getDeviceCount() > 0);
    }
    if (present_) {
        sensors.setResolution(12);
        sensors.setWaitForConversion(false); // nao-bloqueante
        requestConversion(0);
    }
    return present_;
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
        return -1;                // falha de comunicacao
    }
    tempC = t;
    hasValidTemp_ = true;
    lastValidTemp_ = t;
    requestConversion(nowMs);
    return 1;
}
