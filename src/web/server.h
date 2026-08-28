#pragma once
// Web server: / (dashboard), /json (payload), /control (ON/OFF).
// Referencia: FR-API-001..004, FR-UI-007, NFR-TIM-003, NFR-MEM-004
#include <ESP8266WebServer.h>
#include "thermal_fsm.h"
#include "sensor/temperature.h"
#include "metrics/system_health.h"
#include "web/history.h"

class ThermalServer {
public:
    void begin();
    void handle();

    void setFsm(thermocore::ThermalFsm* fsm) { fsm_ = fsm; }
    void setSensor(TemperatureSensor* s) { sensor_ = s; }
    void setHealth(SystemHealth* h) { health_ = h; }
    void setTrend(TrendBuffer* t) { trend_ = t; }

private:
    void handleRoot();
    void handleJson();
    void handleControl();
    void buildJson(char* buf, size_t len);

    ESP8266WebServer server_;
    thermocore::ThermalFsm* fsm_ = nullptr;
    TemperatureSensor* sensor_ = nullptr;
    SystemHealth* health_ = nullptr;
    TrendBuffer* trend_ = nullptr;
};
