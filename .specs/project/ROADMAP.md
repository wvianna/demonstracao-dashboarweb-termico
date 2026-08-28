# ROADMAP — Sistema de Monitoramento e Controle Térmico ESP8266

Fases ordenadas por risco (primeiro contrato e partes de segurança, depois integração e validação física).

## Fase 0 — Fundação (especificação) ✅

- [x] `docs/descricao.txt` recebida (fonte normativa).
- [x] Estrutura `.specs/` criada (project, codebase, feature).
- [x] `features/controle-termico/spec.md`, `context.md`, `design.md`, `tasks.md`.
- [x] Decisões de comportamento resolvidas com o usuário (ver `context.md`).

## Fase 1 — Firmware core (host/build)

- [ ] T-001 Scaffold PlatformIO (`platformio.ini`, pastas, README).
- [ ] T-002 HAL de pinos/constantes.
- [ ] T-003 Scan OneWire + detecção do sensor no boot.
- [ ] T-004 Amostragem 1 s não-bloqueante + falha/recuperação.
- [ ] T-005 Segurança 80 °C + intertravamento.
- [ ] T-006 Agendador de buzzer (150 ms/2 s e 300 ms/5 s).
- [ ] T-007 Controle PWM da carga (ON/OFF).

## Fase 2 — Rede e API

- [ ] T-008 WiFi AP + DHCP + IP fixo 192.168.4.1.
- [ ] T-009 Web server + `/json` + `/control`.
- [ ] T-010 Métricas de saúde (idle, RAM, flash).

## Fase 3 — Dashboard

- [ ] T-011 Dashboard responsivo (HTML/CSS/JS) conforme `design.md`.

## Fase 4 — Integração e validação física (HIL)

- [ ] T-012 Integração + build do alvo + orçamento de memória.
- [ ] T-013 Validação em bancada: sensor, buzzer, resistência, watchdog, uptime 24 h.

## Riscos principais de roadmap

- Validação elétrica do driver da resistência/buzzer depende de hardware em bancada (`A CONFIRMAR`).
- Comunicação com sensor já validada em `TARGET.md` (ROM `28 FF E2 03 B4 16 05 C2` em GPIO 4).
