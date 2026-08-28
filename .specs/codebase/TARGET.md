# TARGET — Alvo de hardware

## Plataforma

- **MCU**: Espressif ESP8266EX (SoC, núcleo Tensilica LX106, 32-bit, ~80 MHz; 160 MHz A CONFIRMAR).
- **Placa**: NodeMCU v2 (módulo ESP-12E), alimentação 5 V (USB) com regulação 3.3 V na placa.
- **Níveis lógicos**: 3.3 V nativos. **Não** usar entradas/saídas em 5 V.
- **Memória**: flash 4 MB (módulo ESP-12E), RAM ~80 KB dados / ~96 KB instruções (heap disponível na prática ~40–50 KB).

## Mapa de pinos (normativo — `docs/descricao.txt`)

| Componente | Pino Físico | GPIO / IO | Nível | Função técnica |
| --- | --- | --- | --- | --- |
| Sensor DS18B20 | D2 | GPIO 04 | 3.3 V | Entrada de dados via protocolo OneWire |
| Buzzer de alarme | D0 | IO 16 | 3.3 V | Saída digital para sinalização sonora |
| Resistência de aquecimento | D1 | IO 05 | 3.3 V | Saída PWM (0–1023, 10 bits) |

### Observações de hardware (validadas / a validar)

- **GPIO 4 (D2)** — DS18B20 confirmado por varredura multi-pino no MCU: ROM `28 FF E2 03 B4 16 05 C2`, devices=1, presence=1. Leituras `t=-127.0`/`dev=0` ocasionais = contato instável, não erro de pinagem.
- **GPIO 16 (D0)** — buzzer: saída digital simples; não possui capacidade PWM/hardware interrupt; ok para sinalização.
- **GPIO 5 (D1)** — resistência: **PWM 10 bits (0–1023)** via `analogWrite()` do core ESP8266. Acionamento direto do GPIO vs driver (MOSFET/transistor) **A CONFIRMAR em bancada (B-002)** — impacto elétrico, não de firmware.

## Periféricos

- **OneWire (DS18B20)**: barramento 1-Wire, scan de ROM no boot (one-shot), resolução 9–12 bit (padrão 12), leitura em °C. Biblioteca: `OneWire` + `DallasTemperature`.
- **PWM**: hardware PWM do ESP8266 (software PWM do core Arduino, base ~1 kHz, resolução 10 bits).
- **Wi-Fi**: SoC integrado; modo AP + DHCP + TCP/IP.

## Clock e watchdog

- Watchdog do ESP8266 (software) exige loop responsivo: **proibido `delay()` bloqueante e ISR/timer para amostragem** (ver `NFR-CONC-001`).
- Clock 80 MHz padrão; 160 MHz opcional (A CONFIRMAR — B-001).

## Ambientes de validação

- **HOST**: lógica de estados/agendamento em testes unitários.
- **BUILD**: compilação PlatformIO para `nodemcuv2`.
- **HIL/BANCADA**: NodeMCU + DS18B20 disponível; buzzer e resistência a confirmar (B-002/B-003).
