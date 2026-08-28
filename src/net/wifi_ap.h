#pragma once
// Modo Access Point aberto + IP fixo + DHCP (FR-NET-001..006).
class WifiAP {
public:
    bool begin();
    const char* ssid() const { return ssid_; }
private:
    char ssid_[32];
};
