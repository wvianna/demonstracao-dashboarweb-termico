# PROJECT — Sistema de Monitoramento e Controle Térmico ESP8266

## Objetivo

Entregar uma solução de **monitoramento e controle térmico de baixo custo** para aplicações industriais/residenciais com alta confiabilidade em hardware limitado. O ESP8266 (NodeMCU v2) atua como nó central de rede (Access Point) e expõe um **dashboard web responsivo** para visualizar a temperatura, acionar a carga de aquecimento e sinalizar condições de risco (≥ 80 °C).

## Fontes de intenção

| Fonte | Papel |
| --- | --- |
| `docs/descricao.txt` | Especificação técnica de entrada (hardware, rede, controle, dashboard, compilação) — **fonte normativa** |
| `docs/descricao.txt` → seção 5 | Requisitos da interface (detalhamento em `features/controle-termico/design.md`) |
| `.github/skills/sdd-embarcado/` | Processo de especificação (esta topologia `.specs/`) |

## Escopo

**Dentro:**

- Firmware ESP8266 (PlatformIO + Arduino): scan OneWire, amostragem 1 s não-bloqueante, segurança 80 °C, alarme sonoro, controle PWM da carga, WiFi AP + DHCP + IP fixo, web server (`/`, `/json`, `/control`), métricas de saúde.
- Dashboard web embutido (HTML/CSS/JS) servido pelo próprio ESP8266, com polling AJAX.
- Validação: build para o alvo e testes em bancada/HIL.

**Fora de escopo (v1):**

- Criptografia de rede (WPA2) — rede aberta por requisito de projeto (ver `NFR-SEC-001`).
- Persistência de dados (EEPROM/SPIFFS) — estado volátil.
- Controle PID / regulação automática de temperatura (apenas ON/OFF manual com proteções).
- Múltiplos sensores, multi-cliente concorrente robusto, OTA, MQTT.
- Modo estação (STA) para conexão a rede Wi-Fi existente.

## Público

Operadores técnicos da planta/local; desenvolvedores e agentes de IA que implementarão o firmware conforme `features/controle-termico/`.

## Métricas de sucesso

- Build para o alvo sem warnings e com flash/RAM dentro dos limites (`NFR-MEM-*`).
- Todos os critérios de aceite `CA-*` em `spec.md` com `PASS` em bancada/HIL (ou pendência registrada).
- Uptime contínuo ≥ 24 h sem reset/watchdog em operação normal (validação em bancada).
