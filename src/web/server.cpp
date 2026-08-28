#include "server.h"
#include "dashboard.h"
#include <Arduino.h>
#include <cstdio>
#include <cstring>

using namespace thermocore;

static const char* alarmName(AlarmKind a) {
    switch (a) {
        case AlarmKind::NONE: return "NONE";
        case AlarmKind::TEMP_HIGH: return "TEMP_HIGH";
        case AlarmKind::SENSOR_FAIL: return "SENSOR_FAIL";
    }
    return "NONE";
}

static const char* buzzerName(BuzzerMode m) {
    switch (m) {
        case BuzzerMode::NONE: return "NONE";
        case BuzzerMode::ALARM_TEMP: return "ALARM_TEMP";
        case BuzzerMode::FAIL_SENSOR: return "FAIL_SENSOR";
    }
    return "NONE";
}

void ThermalServer::begin() {
    server_.on("/", HTTP_GET, [this]() { handleRoot(); });
    server_.on("/json", HTTP_GET, [this]() { handleJson(); });
    server_.on("/control", HTTP_POST, [this]() { handleControl(); });
    server_.begin(); // porta 80 (FR-NET-005)
}

void ThermalServer::handle() {
    server_.handleClient();
}

void ThermalServer::handleRoot() {
    server_.send_P(200, "text/html; charset=utf-8", DASHBOARD_HTML);
}

void ThermalServer::buildJson(char* buf, size_t len) {
    const FsmOutput& o = fsm_->output();
    const bool tv = sensor_->hasValidTemp();
    char tbuf[16];
    if (tv) snprintf(tbuf, sizeof(tbuf), "%.1f", sensor_->lastValidTemp());
    else    strlcpy(tbuf, "null", sizeof(tbuf));

    int used = snprintf(buf, len,
        "{\"temp\":%s,\"tempValid\":%s,\"state\":\"%s\",\"pwm\":%d,"
        "\"load\":%s,\"alarm\":\"%s\",\"buzzer\":\"%s\",\"sensorPresent\":%s,"
        "\"idle\":%.1f,\"heapFree\":%u,\"flashUsedPct\":%.1f,\"uptime\":%lu,\"trend\":[",
        tbuf, tv ? "true" : "false", fsm_->stateName(), o.pwm,
        o.loadRequested ? "true" : "false",
        alarmName(o.alarm), buzzerName(o.buzzer),
        o.sensorPresent ? "true" : "false",
        health_->idlePercent(), (unsigned)health_->heapFree(),
        health_->flashUsedPercent(), (unsigned long)(millis() / 1000UL));

    char* p = buf + used;
    int remain = (int)len - used;
    const int n = trend_->size();
    for (int i = 0; i < n && remain > 8; i++) {
        int w = snprintf(p, remain, "%s%.1f", i ? "," : "", trend_->at(i));
        p += w;
        remain -= w;
    }
    snprintf(p, remain, "]}");
}

void ThermalServer::handleJson() {
    char buf[2048]; // buffer estatico (NFR-MEM-004)
    buildJson(buf, sizeof(buf));
    server_.send(200, "application/json", buf);
}

void ThermalServer::handleControl() {
    bool on = server_.hasArg("on") &&
              (server_.arg("on") == "1" || server_.arg("on") == "true");
    const bool accepted = on ? fsm_->requestOn() : fsm_->requestOff();
    char buf[128];
    snprintf(buf, sizeof(buf),
             "{\"accepted\":%s,\"on\":%s,\"pwm\":%d}",
             accepted ? "true" : "false", on ? "true" : "false",
             fsm_->output().pwm);
    server_.send(200, "application/json", buf);
}
