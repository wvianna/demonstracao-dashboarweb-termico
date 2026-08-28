# HANDSOFF — Controle Térmico ESP8266

> Objetivo para o próximo agente/sessão: validar em bancada (HIL) e encerrar a T-013.

## Estado atual

- Firmware implementado e compilando (Fases 1–3 concluídas): `pio run -e nodemcuv2` → SUCCESS (Flash ≈38,2%, RAM 36,3%).
- Testes HOST passando: `pio test -e native` → 10/10.
- Dashboard validado em navegador com mock (fontes embutidas, estados, ON/OFF).
- **Validação HIL (NodeMCU `/dev/ttyUSB0`, hardware montado):** sensor detectado (ROM `28FFE203B41605C2`); AP/DHCP/HTTP OK; carga ON PWM 1023 persistiu com debounce (D-010); fail-safe confirmado (falha → PWM=0 + `FAIL_SENSOR` + buzzer 300ms/5s). Intervalo de amostragem 1,5 s (D-011).
- **Pendência HIL restante:** aquecimento real ≥80 °C (CA-SAF-001), uptime 24 h (CA-NFR-REL-001), confirmação sonora do buzzer pelo operador.
- **Achado importante:** sensor com falhas intermitentes em rajadas (contato instável) — recomenda-se corrigir fiação/pull-up 4,7 kΩ; firmware já tolera glitches únicos via debounce.

## Objetivo restante

- **T-013 — Pendências de validação (HIL):**
  1. Gravação e boot já validados; AP/DHCP/HTTP e sensor já validados.
  2. **Aquecer ≥ 80 °C** (ou simular) → `ALARM_TEMP` (PWM=0, buzzer 150ms/2s) — CA-SAF-001, NFR-TIM-002.
  3. Medir **jitter de amostragem (1,5 s ±150 ms)** e **uptime ≥ 24 h** — CA-NFR-TIM-001, CA-NFR-REL-001.
  4. **Confirmação sonora do buzzer** (GPIO16/D0) pelo operador durante `FAIL_SENSOR` (300ms/5s) e `ALARM_TEMP` (150ms/2s).
  5. Medir o PWM/tensão no GPIO5 (D1) durante ON (1023 ≈ 3,3 V) e verificar driver elétrico (B-002).
- Confirmar clock 80 vs 160 MHz (B-001) antes do build final se houver problema de margem.

## Arquivos relevantes

- `src/main.cpp` — loop; `lib/thermocore/src/thermal_fsm.cpp` — FSM; `src/sensor/temperature.cpp`; `src/web/server.cpp`; `src/web/dashboard.html` + `dashboard.h` (gerado); `platformio.ini`.
- Spec: `.specs/features/controle-termico/spec.md`; decisões: `context.md`; estado: `project/STATE.md`; resumo: `SUMMARY.md`.

## Decisões tomadas (importantes para o validador)

- Buzzer falha de sensor = 300ms/5s; térmico = 150ms/2s (D-002).
- Após desarme, carga permanece OFF até novo ON (D-005); recuperação automática de leituras (D-006).
- `POST /control?on=1|0` com resposta `accepted` (D-007).
- `SAFE_STOP` (sensor ausente no boot) exige reboot para recuperar — esperado (não é bug).

## Comandos já executados (resultado)

- `pio run -e nodemcuv2` → SUCCESS (398.991 B flash).
- `pio test -e native` → 10/10 PASSED.
- `python3 tools/gen_dashboard.py` → gera `src/web/dashboard.h` (rodado automaticamente no build).
- `pio run -t upload --upload-port /dev/ttyUSB0` → **SUCCESS** (403.280 B gravados, hash verificado).
- Captura serial (reset via DTR/RTS + leitura 6–8s) → boot estável: `[boot] DS18B20 detectado ROM=28FFE203B41605C2` / `[boot] AP: ESP8266_807D3A101026 -> http://192.168.4.1`.
- HTTP via AP (Wi-Fi de teste, internet pela cabeada): `/` HTTP 200, `/json` OK, `POST /control?on=1|0` aceita/rejeita conforme intertravamento.

## Bloqueios de ambiente

- Porta serial: não rodar `pio device monitor` simultâneo à gravação (lock exclusivo).
- **Sensor com contato instável** (falhas em rajadas) — recomenda-se corrigir fiação/pull-up 4,7 kΩ do DS18B20 (D2/GPIO4) para estabilidade; o firmware tolera glitches únicos via debounce (D-010).
- Validação HTTP pode ser feita pela própria máquina de dev via Wi-Fi (internet permanece pela cabeada `enp1s0`).

## Próximos passos

1. Gravar firmware em NodeMCU (seguir acima).
2. Executar checklist HIL da T-013 e registrar `CA-*` com PASS/PENDENTE.
3. Atualizar `STATE.md`, `SUMMARY.md` e a matriz de rastreabilidade em `spec.md` §6.

## Critério para considerar concluído

Todos os `CA-*` em hardware com **PASS** (ou `PENDENTE` com risco registrado), uptime 24h sem reset, e documentação atualizada.
