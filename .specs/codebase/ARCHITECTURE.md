# ARCHITECTURE — Arquitetura do firmware

## Visão geral

Firmware monolítico em loop principal (sem RTOS), agendamento **não-bloqueante** por `millis()`. O ESP8266 opera como Access Point e serve um dashboard web embutido (HTML/CSS/JS em PROGMEM) com API JSON.

```mermaid
flowchart LR
    subgraph FW["Firmware ESP8266"]
        SNS[Driver DS18B20<br/>OneWire scan + leitura 1,5s]
        SAF[Lógica de segurança<br/>80°C / intertravamento]
        ALM[Agendador de buzzer<br/>150ms-2s | 300ms-5s]
        CTL[Controle PWM da carga<br/>ON/OFF 0-1023]
        MET[Métricas<br/>idle / RAM / flash]
        SVR[Web Server :80<br/>/ /json /control]
        AP[WiFi AP + DHCP<br/>192.168.4.1/24]
    end
    DASH[Dashboard web<br/>AJAX polling /json]
    SNS --> SAF
    SAF --> CTL
    SAF --> ALM
    SNS --> SVR
    MET --> SVR
    CTL --> SVR
    SVR --> DASH
    DASH -- POST /control --> SVR
    AP --> SVR
```

## Máquina de estados (controle de carga e segurança)

```mermaid
stateDiagram-v2
    [*] --> BOOT
    BOOT --> SCAN_ONEWIRE
    SCAN_ONEWIRE --> SAFE_STOP: sensor não detectado
    SCAN_ONEWIRE --> MONITORING: sensor detectado

    SAFE_STOP --> MONITORING: sensor detectado (recuperação)
    MONITORING --> HEATER_ON: comando ON (intertravamento ok)
    HEATER_ON --> MONITORING: comando OFF
    MONITORING --> ALARM_TEMP: temp ≥ 80°C
    HEATER_ON --> ALARM_TEMP: temp ≥ 80°C
    MONITORING --> FAIL_SENSOR: falha de leitura
    HEATER_ON --> FAIL_SENSOR: falha de leitura
    ALARM_TEMP --> MONITORING: temp < 80°C (leitura ok)
    FAIL_SENSOR --> MONITORING: leitura válida (recuperação)

    note right of SAFE_STOP
      Carga OFF; alerta; buzzer 300ms/5s
      se falha; aguarda sensor
    endnote
    note right of ALARM_TEMP
      PWM=0; buzzer 150ms/2s;
      carga permanece OFF após saída
    endnote
    note right of FAIL_SENSOR
      PWM=0; buzzer 300ms/5s;
      alerta crítico no dashboard
    endnote
```

## Módulos (diretório sugerido em `src/`)

| Módulo | Responsabilidade | Arquivo sugerido |
| --- | --- | --- |
| HAL de pinos | Constantes de pinagem/níveis | `src/hal/pins.h` |
| Sensor | Scan OneWire + leitura 1,5 s não-bloqueante (debounce N=3 — D-010) | `src/sensor/temperature.h/.cpp` |
| Segurança/controle | FSM de segurança + carga | `src/control/thermal_fsm.h/.cpp` |
| Alarme | Agendador do buzzer (150 ms/2 s, 300 ms/5 s) | `src/alarm/buzzer.h/.cpp` |
| Rede | AP + DHCP + IP fixo | `src/net/wifi_ap.h/.cpp` |
| Web | Rotas `/`, `/json`, `/control` | `src/web/server.h/.cpp` |
| Métricas | Idle, heap, flash | `src/metrics/system_health.h/.cpp` |
| Dashboard | HTML/CSS/JS embutido (PROGMEM) | `src/web/dashboard.h` |

## Contexto de execução

- **Loop principal único**: `loop()` processa as tarefas periódicas agendadas por `millis()`:
  - Sensor: 1,5 s ±150 ms (sem ISR/timer — `NFR-TIM-001`); falha confirmada após 3 leituras consecutivas (D-010).
  - Buzzer: tick fino (~10 ms) para os ciclos 150 ms/2 s e 300 ms/5 s.
  - Web server: `server.handleClient()` a cada iteração.
  - Métricas/idle: janela de 1 s (D-009).
- **Nenhuma ISR de usuário**; sem `delay()` bloqueante.
- Acesso compartilhado: sem locks (execução cooperativa única); o stack Wi-Fi é cooperativo e responde entre iterações.

## Orçamento de memória

| Item | Estimativa | Limite |
| --- | --- | --- |
| Binário (flash) | ≤ 1.5 MB (meta) | 4 MB |
| Buffer JSON `/json` | 1× 1–2 KB estático (pré-alocado) | heap OK |
| Janela de tendência | 120 × float (2 B? usar int16 ×10) ≈ 240 B | RAM |
| Heap livre em operação | ≥ 10 KB | verificado via `/json` |

- **Sem alocação dinâmica** em caminho de tempo real; buffers globais/estáticos.

## Alternativas rejeitadas

- **RTOS (FreeRTOS)**: desnecessário para carga de trabalho; loop cooperativo atende e simplifica determinismo.
- **delay() + ISR para amostragem**: rejeitado — risco de reset por watchdog e bloqueio do web server (normativo na descrição).
- **Gráfico com escala automática**: rejeitado — requisito de percepção imediata do threshold (escala fixa 20–90 °C).
