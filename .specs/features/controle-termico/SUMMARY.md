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
| HIL/BANCADA | T-013 (parcial) | **PARCIAL**: gravação OK (403.280 B, hash verificado); boot estável; AP ativo `ESP8266_807D3A101026`; sensor ausente → `SAFE_STOP` (CA-SNS-001 PASS). HTTP via AP não executado (decisão do usuário). Demais cenários PENDENTES (ver `HANDSOFF.md`) |

## Critérios de aceite — status

- `CA-*` de lógica (FSM/buzzer) cobertos por testes HOST: **PASS** (`CA-SNS-001`, `CA-SAF-001..003`, `CA-CTL-001..002`, ciclos de alarme).
- `CA-*` de UI verificados em navegador com mock: **PASS** (`CA-UI-001..006`, fontes, cores, tooltips).
- `CA-SNS-001` (sensor ausente → carga bloqueada): **PASS (HIL)** — observado no boot real.
- `CA-NET-*`, `CA-SNS-002..004`, `CA-MET-001`, `CA-API-001..002`, `CA-NFR-TIM-*`, `CA-NFR-REL-001`: **PENDENTE (HIL)** — exigem sensor conectado / cliente no AP / uptime 24h.
- `CA-NFR-MEM-001`: **PASS (BUILD)**; `CA-NFR-MEM-002`: **PASS (estimativa)**; confirmar em bancada.

## Desvios da especificação

- `SPEC_DEVIATION-001..003` — registrados em `STATE.md` (local da lib thermocore, formato `/control`, geração do dashboard com fontes embutidas). Comportamento funcional idêntico.

## Riscos residuais / validação pendente

- Validação HIL (T-013): pinos buzzer/resistência, timings reais (amostragem 1s ±150ms, reação ≤10ms), watchdog, uptime 24h.
- Driver elétrico do buzzer/resistência (B-002) e clock 80 vs 160 MHz (B-001).
- `SAFE_STOP` (sensor ausente no boot) requer reboot para recuperar — comportamento documentado (v1).
