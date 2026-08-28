# Tasks — Controle Térmico ESP8266

> Ordem por risco: contrato/testes antes de integração; validação física ao final. Requisitos referenciados em `spec.md`.

## T-001 — Scaffold do projeto PlatformIO

- **Requisitos:** NFR-COMP-001
- **Onde:** raiz do repo (`platformio.ini`, `src/`, `test/`, `.gitignore`, `.gitattributes`, README)
- **Depende de:** nenhum
- **Reutiliza:** padrões PlatformIO
- **Feito quando:** `pio run -e nodemcuv2` compila um "blink"/skeleton; versões registradas.
- **Testes:** build; nivel `BUILD`.
- **Gate:** `pio run -e nodemcuv2` sem erros.

## T-002 — HAL de pinos e constantes

- **Requisitos:** (base para FR-SNS, FR-CTL, FR-ALM)
- **Onde:** `src/hal/pins.h`
- **Depende de:** T-001
- **Reutiliza:** `TARGET.md`
- **Feito quando:** constantes `PIN_SENSOR_TEMP=4`, `PIN_BUZZER=16`, `PIN_LOAD=5`, `PWM_MAX=1023`, threshold `TEMP_ALERT=80.0` definidas e usadas.
- **Testes:** revisão; `BUILD`.
- **Gate:** include compila.

## T-003 — Scan OneWire + detecção no boot

- **Requisitos:** FR-SNS-001, CA-SNS-001
- **Onde:** `src/sensor/temperature.{h,cpp}`
- **Depende de:** T-002
- **Reutiliza:** libs `OneWire` + `DallasTemperature`; padrão `scanOneWirePins()` one-shot (repo memory).
- **Feito quando:** scan no `setup()`; sem dispositivo → estado `SAFE_STOP`; com dispositivo → `MONITORING` e ROM registrada em log.
- **Testes:** BANCADA (sensor conectado/desconectado); `BUILD`.
- **Gate:** boot com sensor removido mantém carga OFF.

## T-004 — Amostragem 1 s não-bloqueante + falha/recuperação

- **Requisitos:** FR-SNS-002..005, NFR-TIM-001, NFR-CONC-001, CA-SNS-002..004, CA-NFR-TIM-001
- **Onde:** `src/sensor/temperature.{h,cpp}`, loop em `main.cpp`
- **Depende de:** T-003
- **Reutiliza:** agendador `millis()` (sem `delay()`/ISR)
- **Feito quando:** leitura a cada 1 s ±150 ms; falha detectada (dev=0 / `-127` / timeout) → evento para a FSM; recuperação automática.
- **Testes:** HOST (teste do agendador com clock simulado) + BANCADA (timestamps).
- **Gate:** `pio test` (host) + log de timestamps coerente.

## T-005 — Segurança 80 °C + intertravamento

- **Requisitos:** FR-SAF-001..004, NFR-TIM-002, CA-SAF-001..003
- **Onde:** `src/control/thermal_fsm.{h,cpp}`
- **Depende de:** T-004
- **Reutiliza:** máquina de estados (`ARCHITECTURE.md`)
- **Feito quando:** temp ≥80 °C ou falha → PWM=0 na mesma iteração; ON rejeitado em risco; carga permanece OFF após desarme (D-005).
- **Testes:** HOST (tabela de transições da FSM) + BANCADA (aquecimento/simulação).
- **Gate:** testes da FSM passam; reação medida ≤ 10 ms.

## T-006 — Agendador de buzzer

- **Requisitos:** FR-ALM-001..004, CA-SAF-001, CA-SNS-003
- **Onde:** `src/alarm/buzzer.{h,cpp}`
- **Depende de:** T-002, T-005
- **Reutiliza:** agendador `millis()` (tick 10 ms)
- **Feito quando:** ciclos 150 ms/2 s (térmico) e 300 ms/5 s (falha de sensor — D-002) corretos; buzzer cessa ao fim da condição.
- **Testes:** HOST (agendador) + BANCADA (cronômetro nos ciclos).
- **Gate:** ciclos medidos conforme especificado.

## T-007 — Controle PWM da carga (ON/OFF)

- **Requisitos:** FR-CTL-001..004, CA-CTL-001..002
- **Onde:** `src/control/thermal_fsm.{h,cpp}` (integra com FSM)
- **Depende de:** T-005
- **Reutiliza:** `analogWrite()` do core ESP8266 (PWM 10 bits)
- **Feito quando:** ON → PWM=1023 (D-001); OFF → 0; intertravamento respeitado; estado real exposto.
- **Testes:** HOST (FSM) + BANCADA (medir tensão/PWM no GPIO5).
- **Gate:** medição do PWM coerente com comando.

## T-008 — WiFi AP + DHCP + IP fixo

- **Requisitos:** FR-NET-001..006, NFR-SEC-001, CA-NET-001..002
- **Onde:** `src/net/wifi_ap.{h,cpp}`
- **Depende de:** T-001
- **Reutiliza:** `ESP8266WiFi` (softAP), SSID a partir do MAC
- **Feito quando:** AP aberto `ESP8266_<6hex>`; IP fixo `192.168.4.1/24`; DHCP ativo (pool D-008).
- **Testes:** BANCADA (conectar cliente, checar IP/site).
- **Gate:** cliente obtém IP `192.168.4.x` e acessa `http://192.168.4.1`.

## T-009 — Web server + `/json` + `/control`

- **Requisitos:** FR-API-001..004, NFR-TIM-003, NFR-MEM-004, CA-API-001..002
- **Onde:** `src/web/server.{h,cpp}`
- **Depende de:** T-005, T-007, T-008
- **Reutiliza:** `ESP8266WebServer`; buffer JSON estático (1 KB)
- **Feito quando:** `/` serve o dashboard; `GET /json` retorna payload completo; `POST /control` (D-007) liga/desliga com resposta de aceite/rejeição; resposta `/json` ≤ 100 ms.
- **Testes:** BANCADA (`curl`); HOST (serialização JSON pura, se extraível).
- **Gate:** `curl http://192.168.4.1/json` válido; `curl -X POST .../control?on=1` reflete estado.

## T-010 — Métricas de saúde

- **Requisitos:** FR-MET-001..003, CA-MET-001, NFR-MEM-002
- **Onde:** `src/metrics/system_health.{h,cpp}`
- **Depende de:** T-009
- **Reutiliza:** `ESP.getFreeHeap()`, `ESP.getSketchSize()`, `ESP.getFreeSketchSpace()`; cálculo de idle (janela 1 s — D-009)
- **Feito quando:** idle %, heap livre KB e flash ocupada % calculados e expostos via `/json`.
- **Testes:** BANCADA (comparar com valores reportados).
- **Gate:** métricas coerentes no `/json`.

## T-011 — Dashboard responsivo (HTML/CSS/JS)

- **Requisitos:** FR-UI-001..009, NFR-UI-001, CA-UI-001..006
- **Onde:** `src/web/dashboard.h` (conteúdo em PROGMEM), assets estáticos
- **Depende de:** T-009
- **Reutiliza:** direção estética "painel de instrumentação industrial" (`design.md` §5)
- **Feito quando:** gauge + número + gráfico (escala fixa 20–90 °C, 120 pontos, grade cinza) + botão ON/OFF com cores + métricas + tooltips + alertas diferenciados; polling AJAX 1 s in-flight único; sem rolagem desktop; pt-BR.
- **Testes:** BANCADA (browser + `curl`); inspeção visual conforme `CA-UI-*`.
- **Gate:** todos os `CA-UI-*` verificados.

## T-012 — Integração + build do alvo + orçamento de memória

- **Requisitos:** NFR-MEM-001..004, NFR-COMP-001, CA-NFR-MEM-001
- **Onde:** repo completo
- **Depende de:** T-005..T-011
- **Reutiliza:** `pio size`
- **Feito quando:** build limpo (zero warnings); flash ≤ 4 MB (meta ≤ 1.5 MB); heap ≥ 10 KB; sem alocação dinâmica no caminho de tempo real.
- **Testes:** BUILD.
- **Gate:** `pio run -e nodemcuv2` + `pio size` registrado; `CA-NFR-MEM-001` PASS.

## T-013 — Validação em bancada (HIL)

- **Requisitos:** NFR-REL-001, CA-NFR-REL-001 + re-verificação dos `CA-*` em hardware
- **Onde:** bancada (NodeMCU + DS18B20 + buzzer + resistência)
- **Depende de:** T-012
- **Reutiliza:** procedimentos das tarefas T-003..T-011
- **Feito quando:** todos os `CA-*` com `PASS` em hardware (ou `PENDENTE` com risco registrado); uptime 24 h sem reset.
- **Testes:** BANCADA/HIL (desconexão de sensor, aquecimento ≥80 °C, watchdog, uptime).
- **Gate:** relatório de validação com evidências por `CA-*`.

## Entregáveis e aceite

**Arquivos esperados (ao final):**

- Código: `src/**` (hal, sensor, control, alarm, net, web, metrics, dashboard), `main.cpp`.
- Testes: `test/**` (host: agendador, FSM, serialização).
- Docs: `README.md` (configurar/compilar/gravar/testar), `.specs/**` atualizado, `STATE.md`, `SUMMARY.md`.

**Compilação (configuração):**

- `pio run -e nodemcuv2` — framework Arduino, board `nodemcuv2`, monitor 115200.

**Testes e nível de evidência:**

- `pio test -e native` (ou host) — lógica/estados (`HOST`).
- `pio run -e nodemcuv2` — build do alvo (`BUILD`).
- `pio run -t upload` + `pio device monitor` — validação física (`BANCADA`/`HIL`).
  - Atenção: não rodar monitor e gravação simultâneos (lock da porta serial).

**Critérios de aceite rastreados:** todos os `CA-*` de `spec.md` §5, com limites de timing (1 s ±150 ms; PWM=0 ≤10 ms; `/json` ≤100 ms), memória (flash ≤4 MB/meta ≤1.5 MB; heap ≥10 KB) e uptime (≥24 h).

**Pendências / riscos residuais:**

- Validação elétrica do driver do buzzer/resistência (B-002) e disponibilidade da bancada (B-003) → responsável: validação física em hardware.
- Clock 80 vs 160 MHz (B-001) a confirmar antes do build final.

## Status de execução (2026-08-28)

| Tarefa | Nível de gate | Status | Evidência |
| --- | --- | --- | --- |
| T-001 Scaffold | BUILD | ✅ | `pio run -e nodemcuv2` |
| T-002 HAL pins | BUILD | ✅ | compila |
| T-003 Scan OneWire boot | BUILD + HOST | ✅ | build; `CA-SNS-001` via FSM (HOST) |
| T-004 Amostragem 1s + falha/recup. | HOST + BUILD | ✅ | testes FSM (falha/recuperação); build |
| T-005 Segurança 80°C + intertrav. | HOST + BUILD | ✅ | testes FSM (alarme 80°C/limiar exato) |
| T-006 Agendador buzzer | HOST + BUILD | ✅ | testes buzzer (150/2000, 300/5000) |
| T-007 Controle PWM ON/OFF | HOST + BUILD | ✅ | testes FSM (ON/OFF, intertravamento) |
| T-008 WiFi AP + DHCP + IP | BUILD | ✅ | compila (validação HIL pendente) |
| T-009 Web server /json /control | BUILD + UI mock | ✅ | build; navegador com mock stateful |
| T-010 Métricas | BUILD | ✅ | compila; `CA-MET-001` HIL pendente |
| T-011 Dashboard | UI mock | ✅ | navegador (fontes, estados, ON/OFF verde/vermelho) |
| T-012 Build + orçamento | BUILD | ✅ | Flash 38,2% / RAM 36,3% (`CA-NFR-MEM-001` PASS) |
| T-013 Validação HIL | HIL | 🔶 **PARCIAL** | gravado + boot + AP/DHCP/HTTP + sensor detectado + carga ON PWM 1023 (debounce) + fail-safe confirmados. Pendentes: aquecimento ≥80 °C, uptime 24 h, som do buzzer → `HANDSOFF.md` |
