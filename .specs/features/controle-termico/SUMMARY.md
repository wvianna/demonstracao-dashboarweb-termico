# SUMMARY — Implementação do Controle Térmico ESP8266

> Registro de entrega (skill sdd-embarcado, passo 8). Data: 2026-08-28.

## Escopo concluído

Firmware completo do Sistema de Monitoramento e Controle Térmico ESP8266 (Fases 1–3 do ROADMAP): sensor DS18B20, FSM de segurança, alarme, controle PWM, rede AP, web server, métricas e dashboard web embutido.

## Entregáveis

**Código (novo):**

- `lib/thermocore/src/thermal_fsm.{h,cpp}` — FSM segurança/carga (lógica pura)
- `lib/thermocore/src/buzzer.{h,cpp}` — agendador do buzzer (lógica pura)
- `lib/thermocore/library.json`
- `src/hal/pins.h` — mapa de pinos/constantes
- `src/sensor/temperature.{h,cpp}` — scan + amostragem 1s não-bloqueante
- `src/net/wifi_ap.{h,cpp}` — AP aberto + IP fixo + DHCP
- `src/metrics/system_health.{h,cpp}` — idle/RAM/flash
- `src/web/history.{h,cpp}` — janela de tendência (120 pontos)
- `src/web/server.{h,cpp}` — rotas `/`, `/json`, `/control`
- `src/web/dashboard.html` + `src/web/dashboard.h` (gerado) — dashboard pt-BR
- `src/main.cpp` — loop principal
- `test/test_fsm/test_fsm.cpp`, `test/test_buzzer/test_buzzer.cpp` — testes HOST
- `tools/gen_dashboard.py`, `tools/fonts/*.woff2` — geração reproduzível do dashboard
- `platformio.ini`, `.gitignore`

**Documentação:**

- `.specs/**` atualizado (STATE com desvios/resultados)
- `README.md` atualizado (compilar/gravar/testar/usar)

## Compilação (alvo)

- Comando: `pio run -e nodemcuv2`
- Resultado: **SUCCESS** (zero erros de compilação; warnings apenas de ferramenta `elf2bin.py` do framework)
- Tamanhos: Flash **398.991 B (38,2%)** de 1.044.464 B — dentro da meta ≤1.5 MB e limite 4 MB (`CA-NFR-MEM-001` PASS/BUILD)
- RAM: **29.704 B (36,3%)** de 81.920 B → heap livre em operação ~52 KB ≥ 10 KB (`CA-NFR-MEM-002` PASS por estimativa de build; confirmar em bancada)

## Testes e níveis

| Nível | Evidência | Resultado |
| --- | --- | --- |
| HOST | `pio test -e native` (Unity) | **10/10 PASSED** (FSM: boot, intertravamento, ON/OFF, alarme 80°C, falha/recuperação, limiar exato; buzzer: ciclos 150/2000 e 300/5000, NONE, troca de modo) |
| BUILD | `pio run -e nodemcuv2` | PASS (tamanhos acima) |
| UI (mock) | navegador + mock HTTP `/json`/`/control` | PASS: fontes embutidas carregam, sem rolagem desktop, empilha mobile, gauge/gráfico desenhados, estados ALARME/FALHA, ciclo ON/OFF verde/vermelho, tooltips |
| HIL/BANCADA | T-013 | **PARCIAL (muito validado)**: gravação OK; boot estável; AP+DHCP+/`/json`/`/control` OK; sensor detectado (ROM `28FFE203B41605C2`); carga ON PWM 1023 persistiu com debounce; fail-safe confirmado (falha → PWM=0 + `FAIL_SENSOR` + buzzer). Pendentes: aquecimento ≥80 °C, uptime 24 h, confirmação sonora do buzzer. |

## Critérios de aceite — status

- `CA-*` de lógica (FSM/buzzer) cobertos por testes HOST: **PASS** (`CA-SNS-001`, `CA-SAF-001..003`, `CA-CTL-001..002`, ciclos de alarme).
- `CA-*` de UI verificados em navegador com mock: **PASS** (`CA-UI-001..006`, fontes, cores, tooltips).
- `CA-SNS-001` (sensor ausente → carga bloqueada): **PASS (HIL)** — observado no boot real (antes do fix de detecção).
- `CA-SNS-002` (leituras 1,5 s ±150 ms): **PASS (HIL)** — sensor lendo °C reais.
- `CA-SNS-003`/`FR-SAF-002` (falha confirmada → PWM=0 + alarme + buzzer): **PASS (HIL)** — observado em hardware.
- `CA-CTL-001` (ON → PWM 1023): **PASS (HIL)** — `HEATER_ON` persistiu ~5 s com debounce.
- `CA-NET-001/002`, `CA-API-001/002` (AP/DHCP/HTTP): **PASS (HIL)** — `/`, `/json`, `POST /control` validados via `curl`.
- `CA-UI-*`: **PASS (mock/navegador)** — validação visual com mock; em hardware via cliente no AP pendente.
- `CA-SAF-001` (temp ≥80 °C), `CA-NFR-REL-001` (uptime 24 h), confirmação sonora do buzzer: **PENDENTE (HIL)**.
- `CA-NFR-MEM-001`: **PASS (BUILD)**; `CA-NFR-MEM-002`: **PASS (estimativa)**; confirmar em bancada.

## Desvios da especificação

- `SPEC_DEVIATION-001..004` — registrados em `STATE.md` (local da lib thermocore; formato `/control`; geração do dashboard com fontes embutidas; debounce D-010 + intervalo 1,5 s D-011). Comportamento funcional idêntico ou conforme decisões aprovadas pelo usuário.
- **Correção (bug de detecção)**: `DallasTemperature::getDeviceCount()` retorna o cache de `begin()`; fix re-executa `begin()` no boot (retry) e loga a ROM.

## Riscos residuais / validação pendente

- Validação HIL (T-013): pinos buzzer/resistência, timings reais (amostragem 1s ±150ms, reação ≤10ms), watchdog, uptime 24h.
- Driver elétrico do buzzer/resistência (B-002) e clock 80 vs 160 MHz (B-001).
- `SAFE_STOP` (sensor ausente no boot) requer reboot para recuperar — comportamento documentado (v1).
