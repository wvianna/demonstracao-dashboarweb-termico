# STACK — Tecnologias e toolchain

## Stack principal

- **IDE/Build**: PlatformIO Core (`platformio` CLI).
- **Framework**: Arduino para ESP8266 (`framework = arduino`, core `esp8266`).
- **Board**: `nodemcuv2` (NodeMCU v2 / ESP-12E).
- **Linguagem**: C++ (padrão C++11 do core ESP8266).
- **Libraries** (a fixar versões no `platformio.ini`):
  - `OneWire` (paulstoffregen/OneWire)
  - `DallasTemperature` (milesburton/DallasTemperature)
  - `ESP8266WiFi`, `ESP8266WebServer` (core)
  - `ESP8266mDNS` (opcional, não obrigatório)
- **Ferramentas de validação**: `pio test` (host) para testes de lógica/estados; monitor serial `pio device monitor` (115200 baud).

## Parâmetros de compilação (normativos — `docs/descricao.txt`)

- Footprint máximo de flash: **4 MB**.
- Binário otimizado para tempo real; **sem persistência** (sem EEPROM/SPIFFS).
- Politica de warnings: zero warnings no alvo.

## Exemplo de `platformio.ini` (a ajustar na T-001)

```ini
[env:nodemcuv2]
platform = espressif8266
board = nodemcuv2
framework = arduino
monitor_speed = 115200
board_build.filesystem = none
build_flags =
    -DCORE_DEBUG_LEVEL=0
lib_deps =
    paulstoffregen/OneWire@^2.3.8
    milesburton/DallasTemperature@^3.11.0
```

> Versões exatas das dependências serão fixadas após o scaffold (T-001).

## Reprodutibilidade

- Todas as versões (PlatformIO, core `esp8266`, libs) registradas no `platformio.ini`/`platformio.lock`.
- Comando de build documentado no `README.md`: `pio run -e nodemcuv2`.
