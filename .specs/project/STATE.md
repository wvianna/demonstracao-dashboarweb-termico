# STATE — Estado do projeto

> Atualizado ao final de cada iteração. Última atualização: 2026-08-28.

## Estado atual

- **Especificação elaborada** (Fase 0): `spec.md`, `context.md`, `design.md`, `tasks.md` criados.
- **Firmware implementado** (Fases 1–3) e compilando para o alvo; testes HOST passando; dashboard validado em navegador (mock).
  - Build `nodemcuv2`: **SUCCESS** — Flash 398.991 B (38,2% de 1.044.464 B; meta ≤1.5 MB), RAM 29.704 B (36,3% de 81.920 B → heap livre ~52 KB).
  - Testes HOST `native`: **10/10 PASSED** (FSM 6 + buzzer 4).
  - Dashboard: fontes embutidas (Chakra Petch + IBM Plex Sans) carregam offline; sem rolagem em desktop; empilha em mobile; estados ALARME/FALHA e ciclo ON/OFF (verde/vermelho) verificados em navegador.
- **HIL (NodeMCU em `/dev/ttyUSB0`) — validado em hardware montado:**
  - Firmware gravado com sucesso (403.280 B, hash verificado) e **boot estável/reproduzível**.
  - AP ativo: SSID `ESP8266_807D3A101026` (FR-NET-002) → `http://192.168.4.1`; DHCP OK; `/` (HTTP 200), `/json` e `POST /control` respondendo.
  - **Sensor DS18B20 (D2/GPIO4)**: detectado, ROM `28FFE203B41605C2`, lendo °C reais (38–41 °C). **CA-SNS-001/002 PASS** (com retry de `begin()` no boot).
  - **Carga (D1/IO5)**: `POST on=1` → `HEATER_ON` PWM 1023; persistiu ~5 s (debounce segurou glitches) → **CA-CTL-001 PASS**.
  - **Fail-safe**: falha confirmada (3 leituras consecutivas) → PWM=0 + `FAIL_SENSOR` + buzzer `FAIL_SENSOR` → **CA-SNS-003 / FR-SAF-002 PASS**.
  - **Buzzer (D0/IO16)**: modo `FAIL_SENSOR` ativo (300 ms/5 s) durante falha — confirmação sonora/elétrica a cargo do operador.
- **Achado HIL:** o sensor apresenta **falhas intermitentes em rajadas** (3+ leituras consecutivas) mesmo com carga desligada — instabilidade de contato/barramento (sintoma já documentado na memória). O firmware está correto (fail-safe + debounce); recomenda-se **corrigir contato/pull-up 4,7 kΩ** do DS18B20 para estabilidade.
- **Pendente (HIL):** aquecimento real ≥80 °C (CA-SAF-001), uptime ≥24 h (CA-NFR-REL-001), confirmação sonora do buzzer pelo operador.
- Hardware parcialmente validado: sensor DS18B20 confirmado por varredura multi-pino em **D2/GPIO 4** (ROM `28 FF E2 03 B4 16 05 C2`, devices=1, presence=1).

## Decisões registradas

| ID | Decisão | Status |
| --- | --- | --- |
| D-001 | PWM ao ligar a carga: **fixo em 1023 (100%)** | Aprovada (usuário) |
| D-002 | Buzzer em falha de sensor: **300 ms ON / 5 s OFF** (distinto do térmico 150 ms/2 s) | Aprovada (usuário) |
| D-003 | Janela do gráfico de tendência: **120 pontos (~2 min)** | Aprovada (usuário) |
| D-004 | Idioma do dashboard: **pt-BR** | Aprovada (usuário) |
| D-005 | Após desarme de segurança, carga permanece OFF até novo ON explícito (fail-safe) | Implementado (`thermal_fsm.cpp`) |
| D-006 | Recuperação automática de leituras quando o sensor volta a responder | Implementado (`temperature.cpp`/FSM) |
| D-007 | Controle via `POST /control` (`on=1\|0`), resposta com aceite/rejeição | Implementado (`server.cpp`) |
| D-008 | Pool DHCP `192.168.4.2–192.168.4.254` | Padrão `softAP` (não configurado explicitamente) |
| D-009 | Medição de carga idle em janela de **1 s** | Implementado (`system_health.cpp`) |
| D-010 | **Debounce de falha do sensor**: falha só após 3 leituras consecutivas com erro | Implementado (`temperature.cpp`) |
| D-011 | **Intervalo de amostragem: 1,5 s** (era 1 s) | Implementado (`pins.h`/`temperature.cpp`) |

## Desvios de implementação (registrados)

- `SPEC_DEVIATION-001`: FSM de segurança/controle e agendador de buzzer ficam em `lib/thermocore/` (lógica pura, testável em host) em vez de `src/control/` e `src/alarm/` — mesmo comportamento, melhor testabilidade.
- `SPEC_DEVIATION-002`: Endpoint de controle é `POST /control?on=1|0` (query string no corpo/URL), pois o `ESP8266WebServer` simplifica o parsing; comportamento conforme D-007.
- `SPEC_DEVIATION-003`: Dashboard HTML é gerado de `src/web/dashboard.html` + fontes em `tools/fonts/` → `src/web/dashboard.h` via `tools/gen_dashboard.py` (`extra_scripts`). Fontes embutidas em base64 (rede AP sem internet).
- `SPEC_DEVIATION-004`: **Debounce de falha do sensor (D-010)** — desligamento por falha só após 3 leituras consecutivas (~4,5 s), e **intervalo de amostragem 1,5 s (D-011)** — mudanças aprovadas pelo usuário; refletidas em `FR-SNS-002/004`, `FR-SAF-002`, `NFR-TIM-001/004`.
- **Correção (bug)**: detecção do sensor no boot falhava porque `DallasTemperature::getDeviceCount()` retorna o cache de `begin()`; corrigido re-executando `begin()` (re-enumera) no retry + log da ROM. Resultado: sensor detectado (ROM `28FFE203B41605C2`).

## Bloqueios / a confirmar

- **B-001** Clock da CPU: 80 MHz (padrão NodeMCU) vs 160 MHz → confirmar antes do build final.
- **B-002** Driver elétrico do buzzer e da resistência (GPIO direto vs transistor/MOSFET) → validar em bancada; não bloqueia o firmware.
- **B-003** Disponibilidade de bancada para validação HIL (buzzer, resistência, uptime 24 h).

## Lições registradas (repo memory)

- `PIN_SENSOR_TEMP=4` (D2) confirmado; leituras `t=-127.0` ocasionais = contato instável, não erro de pinagem.
- Varredura OneWire deve ser one-shot no boot (não periódica no loop — ruído/instabilidade).
- Monitor serial segura a porta com lock exclusivo: não rodar monitor e gravação simultaneamente.
