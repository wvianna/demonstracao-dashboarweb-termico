#include "wifi_ap.h"
#include "hal/pins.h"
#include <ESP8266WiFi.h>
#include <cstdio>

bool WifiAP::begin() {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    // SSID dinamico derivado do MAC (FR-NET-002): ESP8266_<6 hex>
    snprintf(ssid_, sizeof(ssid_), "ESP8266_%02X%02X%02X%02X%02X%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    WiFi.mode(WIFI_AP);
    IPAddress ip(AP_IP_1, AP_IP_2, AP_IP_3, AP_IP_4);        // 192.168.4.1
    IPAddress gw(AP_IP_1, AP_IP_2, AP_IP_3, AP_IP_4);
    IPAddress mask(255, 255, 255, 0);
    WiFi.softAPConfig(ip, gw, mask); // FR-NET-003 (IP fixo /24)

    // FR-NET-001: rede aberta (NFR-SEC-001) + DHCP ativo por padrao (FR-NET-004)
    return WiFi.softAP(ssid_);
}
