# HANDSOFF — Controle Térmico ESP8266

> Objetivo para o próximo agente/sessão: validar em bancada (HIL) e encerrar a T-013.

## Estado atual

- Firmware implementado e compilando (Fases 1–3 concluídas): `pio run -e nodemcuv2` → SUCCESS (Flash 38,2%, RAM 36,3%).
- Testes HOST passando: `pio test -e native` → 10/10.
- Dashboard validado em navegador com mock (fontes embutidas, estados, ON/OFF).
- **Validação física (HIL) parcial:** firmware gravado no NodeMCU (`/dev/ttyUSB0`), boot estável, AP `ESP8266_807D3A101026` ativo, sensor ausente → `SAFE_STOP` (CA-SNS-001 PASS). HTTP via AP **não** testado (decisão do usuário: não conectar a máquina de dev ao AP).

## Objetivo restante

- **T-013 — Validação em bancada (HIL):**
  1. Gravar e rodar em NodeMCU v2: `pio run -t upload --upload-port /dev/ttyUSB0`.
  2. Conectar à rede `ESP8266_<MAC>` e abrir `http://192.168.4.1` (CA-NET-001/002).
  3. Sensor DS18B20 (D2/GPIO4) já validado; testar desconexão → FAIL_SENSOR (buzzer 300ms/5s, PWM=0) e reconexão → recuperação (CA-SNS-003/004).
  4. Aquecer ≥ 80 °C (ou simular) → ALARM_TEMP (PWM=0 ≤10ms, buzzer 150ms/2s) (CA-SAF-001, NFR-TIM-002).
  5. Medir jitter de amostragem (1s ±150ms) e uptime ≥ 24h (CA-NFR-TIM-001, CA-NFR-REL-001).
  6. Confirmar buzzer/resistência acionados (verificar driver elétrico — B-002).
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
- Captura serial (reset via DTR/RTS + leitura 6–8s) → boot estável: `[boot] sensor NAO detectado - carga bloqueada` / `[boot] AP: ESP8266_807D3A101026 -> http://192.168.4.1`.

## Bloqueios de ambiente

- Porta serial: não rodar `pio device monitor` simultâneo à gravação (lock exclusivo).
- **Não conectar a máquina de dev ao AP do NodeMCU** (decisão do usuário) — validação HTTP deve ser feita por outro cliente (celular/notebook) na rede `ESP8266_807D3A101026`.
- Sensor não detectado no boot atual — verificar fiação/contato do DS18B20 (D2/GPIO4) antes dos cenários de sensor.

## Próximos passos

1. Gravar firmware em NodeMCU (seguir acima).
2. Executar checklist HIL da T-013 e registrar `CA-*` com PASS/PENDENTE.
3. Atualizar `STATE.md`, `SUMMARY.md` e a matriz de rastreabilidade em `spec.md` §6.

## Critério para considerar concluído

Todos os `CA-*` em hardware com **PASS** (ou `PENDENTE` com risco registrado), uptime 24h sem reset, e documentação atualizada.
