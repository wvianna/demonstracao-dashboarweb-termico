# Feature: Controle Térmico ESP8266 — `spec.md`

> Fonte normativa: `docs/descricao.txt`. Processo: `.github/skills/sdd-embarcado`. Decisões: `context.md`. Design: `design.md`.

## 1. Objetivo e fora de escopo

**Objetivo:** monitorar a temperatura via DS18B20, controlar uma carga de aquecimento por PWM (0–1023) de forma segura (limite 80 °C), sinalizar risco por buzzer e expor um dashboard web responsivo servido pelo próprio ESP8266 em modo Access Point.

**Fora de escopo (v1):** criptografia Wi-Fi, persistência (EEPROM/SPIFFS), controle PID, múltiplos sensores, OTA/MQTT, modo STA.

## 2. Atores, estados e eventos

**Atores:**

- **Operador** — interage com o dashboard (liga/desliga carga, observa dados).
- **Firmware** — lógica de monitoramento, segurança, alarme e rede.
- **Sensor DS18B20** — fonte de temperatura (OneWire).
- **Atuadores** — resistência (PWM) e buzzer.

**Estados:** `BOOT`, `SCAN_ONEWIRE`, `SAFE_STOP`, `MONITORING`, `HEATER_ON`, `ALARM_TEMP`, `FAIL_SENSOR` (ver máquina de estados em `design.md`/`ARCHITECTURE.md`).

**Eventos:** sensor detectado, leitura válida, falha de leitura, temp ≥ 80 °C, temp < 80 °C, comando ON, comando OFF, cliente HTTP, polling `/json`.

## 3. Requisitos funcionais

### FW-NET — Rede (AP/DHCP/Web)

| ID | Requisito |
| --- | --- |
| FR-NET-001 | O ESP8266 opera em **modo Access Point (AP) sem criptografia**. |
| FR-NET-002 | SSID dinâmico derivado do MAC do chip: `ESP8266_<6 últimos hex do MAC>`. |
| FR-NET-003 | IP fixo `192.168.4.1`, máscara `255.255.255.0` (`/24`), gateway `192.168.4.1`. |
| FR-NET-004 | Servidor DHCP ativo atribuindo IPs aos clientes. |
| FR-NET-005 | Web Server HTTP ativo na **porta 80**. |
| FR-NET-006 | Os serviços web permanecem responsivos durante toda a operação (amostragem não bloqueia). |

### FW-SNS — Sensor

| ID | Requisito |
| --- | --- |
| FR-SNS-001 | No **setup**, varredura OneWire (scan) obrigatória do barramento para identificar o endereço do DS18B20; **sem detecção, a carga não pode ser acionada** (estado seguro). |
| FR-SNS-002 | Amostragem térmica a cada **1,5 s** com tolerância **±150 ms** (D-011), **não-bloqueante** via `millis()` no loop; **proibido** `delay()` e interrupções/timers de hardware para esta tarefa. |
| FR-SNS-003 | Leitura válida atualiza a temperatura corrente exibida e usada pela segurança. |
| FR-SNS-004 | Falha de leitura **confirmada** (3 leituras consecutivas com erro — debounce D-010) → tratamento de segurança (PWM = 0) + alerta crítico enviado ao dashboard; glitch único é absorvido (mantém o último estado). |
| FR-SNS-005 | Quando o sensor voltar a responder, o sistema **retoma as leituras automaticamente** e limpa o alerta de falha (carga permanece OFF). |

### FW-SAF — Segurança

| ID | Requisito |
| --- | --- |
| FR-SAF-001 | Se temperatura **≥ 80 °C**, executa **desligamento imediato da carga (PWM = 0)**. |
| FR-SAF-002 | Se houver **falha de comunicação confirmada** com o sensor (3 leituras consecutivas com erro — debounce D-010), executa **desligamento (PWM = 0)**, mesmo que o último valor lido fosse seguro. |
| FR-SAF-003 | Enquanto a condição de risco persistir (temp ≥ 80 °C **ou** falha de leitura), a carga permanece **bloqueada** e o comando ON é **rejeitado**. |
| FR-SAF-004 | Após sair da condição de risco (temp < 80 °C e leitura ok), a carga **permanece OFF** até novo comando ON explícito (fail-safe). |

### FW-ALM — Alarme (buzzer)

| ID | Requisito |
| --- | --- |
| FR-ALM-001 | Buzzer em ciclo **150 ms ON / 2 s OFF** enquanto **temp ≥ 80 °C** persistir. |
| FR-ALM-002 | Buzzer em ciclo **300 ms ON / 5 s OFF** durante **falha de comunicação** do sensor (decisão D-002). |
| FR-ALM-003 | O buzzer cessa quando a condição que o originou cessa (temp < 80 °C com leitura ok; ou leitura válida retomada). |
| FR-ALM-004 | Alertas diferenciados (térmico vs falha de sensor) refletidos no dashboard (estado/cor/ícone). |

### FW-CTL — Controle da carga

| ID | Requisito |
| --- | --- |
| FR-CTL-001 | Comandos ON/OFF enviados pelo dashboard controlam a resistência (via PWM). |
| FR-CTL-002 | ON aplica **PWM fixo = 1023 (100%)** (decisão D-001). |
| FR-CTL-003 | **Intertravamento:** ON é rejeitado se sensor ausente, em falha de leitura ou temp ≥ 80 °C; o estado atual é mantido e a rejeição é informada. |
| FR-CTL-004 | O **estado real** da carga (incluindo desarme automático por segurança) é retornado pelo `/json` e refletido no dashboard (indicador verde ativo / vermelho desligado). |

### FW-API — API/JSON

| ID | Requisito |
| --- | --- |
| FR-API-001 | `GET /json` retorna payload JSON com: temperatura, estado da carga, nível PWM, alarme/tipo, métricas de saúde (idle, RAM, flash) e alertas/erros vigentes. |
| FR-API-002 | `POST /control` (`on=1\|0`) liga/desliga a carga; a resposta indica **aceite ou rejeição** (intertravamento). |
| FR-API-003 | Alertas críticos (falha de leitura, temp ≥ 80 °C) presentes no payload do `/json`. |
| FR-API-004 | API stateless (sem sessão/cookies), compatível com polling AJAX. |

### FW-MET — Métricas de saúde

| ID | Requisito |
| --- | --- |
| FR-MET-001 | Calcula a **carga idle** (percentual de tempo ocioso do loop em janela de 1 s — D-009). |
| FR-MET-002 | Reporta **uso de RAM** (heap livre) e **ocupação da flash** (tamanho do sketch/espaço livre). |
| FR-MET-003 | Valores expostos via `/json` e exibidos no cabeçalho do dashboard. |

### UI-DASH — Dashboard web

| ID | Requisito |
| --- | --- |
| FR-UI-001 | Página única **responsiva, tema claro**, sem rolagem em viewport desktop (≥ 1280×800); rolagem vertical permitida em mobile. |
| FR-UI-002 | Temperatura exibida em **Gauge** e **valor numérico em destaque**. |
| FR-UI-003 | **Gráfico de tendência** com escala fixa **20–90 °C** (sem auto-escala) e **grade de fundo cinza claro**; janela de **120 pontos** (D-003). |
| FR-UI-004 | Botão **ON/OFF** com indicadores de estado por cor (verde ativo, vermelho desligado) e bloqueio visual quando intertravado. |
| FR-UI-005 | Cabeçalho com métricas de saúde (idle, RAM, flash). |
| FR-UI-006 | **Tooltips** de ajuda em todos os elementos de controle e visualização. |
| FR-UI-007 | Atualização via **polling assíncrono (AJAX)** ao `/json`, sem refresh total da página. |
| FR-UI-008 | **Alerta visual diferenciado** para temp alta vs falha de sensor (estado crítico). |
| FR-UI-009 | Idioma **pt-BR** (D-004). |

## 4. Requisitos não funcionais

| ID | Requisito |
| --- | --- |
| NFR-TIM-001 | Período de amostragem térmica **1,5 s ± 150 ms** (medido entre inícios de leituras) — D-011. |
| NFR-TIM-004 | Falha de sensor **confirmada** após 3 leituras consecutivas com erro (~4,5 s); glitch único não aciona fail-safe (debounce D-010). |
| NFR-TIM-002 | Reação de segurança: intervalo entre a **leitura crítica** (≥ 80 °C ou falha) e **PWM = 0** ≤ **10 ms** (mesma iteração do loop). |
| NFR-TIM-003 | Resposta HTTP do `/json` ≤ **100 ms** em operação normal (sem falha). |
| NFR-CONC-001 | Nenhum `delay()` bloqueante; **nenhuma ISR/timer** para amostragem/alarme; agendamento por `millis()`. |
| NFR-MEM-001 | Flash do binário ≤ **4 MB** (limite da plataforma); **meta ≤ 1.5 MB**; margem ≥ 40% livre. |
| NFR-MEM-002 | **Heap livre** em operação ≥ **10 KB** (margem mínima), verificado via `/json` e monitor. |
| NFR-MEM-003 | **Sem persistência** (sem EEPROM/SPIFFS); estado volátil. |
| NFR-MEM-004 | **Sem alocação dinâmica** no caminho de tempo real; buffers pré-alocados/estáticos. |
| NFR-REL-001 | Watchdog não dispara em operação normal; **uptime contínuo ≥ 24 h** sem reset (validação em bancada). |
| NFR-UI-001 | Polling `/json` a cada **1 s** com timeout e **requisição in-flight única** (sem acúmulo). |
| NFR-SEC-001 | Rede aberta (sem criptografia) — **aceito por requisito de projeto** (disponibilidade > segurança); risco documentado. |
| NFR-COMP-001 | Build reproduzível via PlatformIO (board `nodemcuv2`, framework Arduino), versões registradas, zero warnings. |

## 5. Critérios de aceitação (DADO / QUANDO / ENTÃO)

| ID | Critério |
| --- | --- |
| CA-NET-001 | DADO placa alimentada e firmware iniciado QUANDO o AP sobe ENTÃO a rede `ESP8266_<6 hex>` aparece sem senha. |
| CA-NET-002 | DADO AP ativo QUANDO um cliente conecta ENTÃO recebe IP `192.168.4.x` via DHCP e acessa `http://192.168.4.1`. |
| CA-SNS-001 | DADO sensor desconectado QUANDO ocorre o boot QUANDO o scan não encontra o dispositivo ENTÃO a carga permanece desabilitada (PWM = 0) e o dashboard indica estado seguro. |
| CA-SNS-002 | DADO sensor conectado QUANDO o sistema está em operação ENTÃO as leituras ocorrem a cada 1,5 s ± 150 ms, sem `delay()`/ISR (evidência: timestamps no `/json` ou serial). |
| CA-SNS-003 | DADO **3 leituras consecutivas** com falha QUANDO ocorrem ENTÃO PWM = 0, alerta crítico no dashboard e buzzer soa em 300 ms/5 s; DADO 1–2 leituras com falha isolada ENTÃO o último estado é mantido (debounce). |
| CA-SNS-004 | DADO sensor volta a responder QUANDO uma leitura válida ocorre ENTÃO o monitoramento é retomado e o alerta de falha é limpo (carga permanece OFF). |
| CA-SAF-001 | DADO temp ≥ 80 °C QUANDO a carga está ON ENTÃO PWM = 0 em ≤ 10 ms e o buzzer soa em 150 ms/2 s. |
| CA-SAF-002 | DADO temp ≥ 80 °C ou falha QUANDO o operador envia ON ENTÃO o comando é rejeitado, a carga permanece OFF e o dashboard indica bloqueio. |
| CA-SAF-003 | DADO temp < 80 °C e leitura ok QUANDO a condição de risco cessa ENTÃO o buzzer para e a carga permanece OFF até novo ON explícito. |
| CA-CTL-001 | DADO sensor ok e temp < 80 °C QUANDO o operador envia ON ENTÃO a carga liga com PWM = 1023 e o indicador fica verde. |
| CA-CTL-002 | DADO carga ON QUANDO o operador envia OFF ENTÃO PWM = 0 e o indicador fica vermelho. |
| CA-API-001 | DADO servidor ativo QUANDO `GET /json` é chamado ENTÃO retorna JSON válido com todos os campos (temp, estado, PWM, alarme, métricas). |
| CA-API-002 | DADO condição de risco vigente QUANDO o payload é gerado ENTÃO o campo de alerta traz o código/tipo do alarme (térmico ou falha). |
| CA-MET-001 | DADO sistema em operação QUANDO `/json` é consultado ENTÃO idle, heap livre e flash ocupada estão presentes e coerentes. |
| CA-UI-001 | DADO dashboard aberto em desktop 1280×800 QUANDO renderizado ENTÃO todo o conteúdo fica visível sem rolagem. |
| CA-UI-002 | DADO dashboard aberto QUANDO a temperatura atualiza ENTÃO gauge, valor numérico e gráfico (escala fixa 20–90 °C, grade cinza) refletem o valor. |
| CA-UI-003 | DADO gráfico de tendência QUANDO mais de 120 amostras são geradas ENTÃO a janela deslizante mantém apenas os últimos 120 pontos. |
| CA-UI-004 | DADO dashboard QUANDO o usuário passa o cursor em controles/visualizações ENTÃO tooltips explicativos aparecem. |
| CA-UI-005 | DADO dashboard aberto QUANDO os dados mudam ENTÃO a atualização ocorre via AJAX, sem refresh da página. |
| CA-UI-006 | DADO estado crítico QUANDO o dashboard está ativo ENTÃO alerta visual diferenciado (temp alta vs falha de sensor) é exibido. |
| CA-NFR-MEM-001 | DADO build do firmware QUANDO compilado ENTÃO o tamanho de flash é ≤ 4 MB e a meta ≤ 1.5 MB é registrada. |
| CA-NFR-MEM-002 | DADO operação contínua QUANDO o heap é monitorado ENTÃO heap livre ≥ 10 KB. |
| CA-NFR-TIM-001 | DADO loop ativo QUANDO a amostragem é executada ENTÃO o jitter de amostragem é ≤ 150 ms. |
| CA-NFR-REL-001 | DADO operação normal QUANDO o uptime é monitorado por 24 h ENTÃO não ocorre reset/watchdog (validação em bancada). |

## 6. Matriz de rastreabilidade requisito → critério → evidência

| Requisito | Critério de aceite | Evidência prevista | Nível |
| --- | --- | --- | --- |
| FR-NET-001..005 | CA-NET-001, CA-NET-002 | Teste de conexão em bancada; leitura de IP/SSID | BANCADA |
| FR-NET-006 | CA-API-001, CA-NFR-REL-001 | Teste de responsividade sob carga | BANCADA |
| FR-SNS-001 | CA-SNS-001 | Boot com sensor removido; log serial | BANCADA |
| FR-SNS-002 | CA-SNS-002, CA-NFR-TIM-001 | Timestamps serial/`/json`; teste host do agendador | HOST + BANCADA |
| FR-SNS-003..005 | CA-SNS-003, CA-SNS-004 | Desconectar/reconectar sensor em bancada | BANCADA |
| FR-SAF-001..004 | CA-SAF-001..003 | Aquecer acima de 80 °C ou simular valor; teste host da FSM | HOST + BANCADA |
| FR-ALM-001..004 | CA-SAF-001, CA-SNS-003, CA-UI-006 | Medir ciclos do buzzer (cronômetro) | BANCADA |
| FR-CTL-001..004 | CA-CTL-001, CA-CTL-002, CA-SAF-002 | Teste host da FSM + interação no dashboard | HOST + BANCADA |
| FR-API-001..004 | CA-API-001, CA-API-002 | `curl http://192.168.4.1/json`; validar schema | BANCADA |
| FR-MET-001..003 | CA-MET-001 | Comparar com `ESP.getFreeHeap()` etc. | BANCADA |
| FR-UI-001..009 | CA-UI-001..006 | Inspeção visual + teste de UI (browser) | BANCADA |
| NFR-TIM-001..003 | CA-NFR-TIM-001, CA-SAF-001 | Medição com osciloscópio/log | BANCADA |
| NFR-MEM-001..004 | CA-NFR-MEM-001..002 | `pio size`; `/json` heap | BUILD + BANCADA |
| NFR-REL-001 | CA-NFR-REL-001 | Uptime 24 h em bancada | BANCADA/HIL |
| NFR-COMP-001 | Build limpo | `pio run -e nodemcuv2` sem warnings | BUILD |

## 7. Premissas, riscos e perguntas bloqueadoras

**Premissas:**

- Hardware disponível para bancada (NodeMCU + DS18B20 confirmados; buzzer/resistência a confirmar).
- Driver elétrico de buzzer/resistência será definido em bancada (não bloqueia firmware).
- Rede aberta é aceitável para o contexto (disponibilidade > segurança).

**Riscos:**

- **R-001** Falha de hardware do sensor intermitente (`t=-127.0`) → mitigado por falha → PWM=0 e alarme.
- **R-002** Reset por watchdog se o loop bloquear → mitigado por proibição de `delay()`/ISR e agendamento `millis()`.
- **R-003** Vazamento de heap em polling/JSON → mitigado por buffers estáticos (`NFR-MEM-004`) e monitoramento de heap.
- **R-004** Rede aberta → risco de acesso não autorizado ao controle; mitigação: ambiente local/industrial, documentado (NFR-SEC-001).

**Perguntas bloqueadoras (ver `context.md`):**

- **B-001** Clock 80 vs 160 MHz.
- **B-002** Driver elétrico do buzzer/resistência.
- **B-003** Disponibilidade de bancada para HIL (buzzer, resistência, uptime 24 h).
