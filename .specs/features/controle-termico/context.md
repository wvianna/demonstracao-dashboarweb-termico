# Context — Decisões e ambiguidades da feature

> Registro das decisões que mudam comportamento/arquitetura (passo 3 da skill). Respostas do usuário em 2026-08-28.

## Decisões aprovadas (usuário)

| ID | Decisão | Impacto |
| --- | --- | --- |
| D-001 | **PWM ao ligar a carga: fixo em 1023 (100%)** — botão ON/OFF binário. | `FR-CTL-002`, `CA-CTL-001` |
| D-002 | **Buzzer em falha de sensor: 300 ms ON / 5 s OFF** (térmico segue 150 ms/2 s). | `FR-ALM-002`, `CA-SNS-003` |
| D-003 | **Janela do gráfico de tendência: 120 pontos (~2 min).** | `FR-UI-003`, `CA-UI-003`, `NFR-MEM` |
| D-004 | **Idioma do dashboard: pt-BR.** | `FR-UI-009` |

## Decisões propostas (A CONFIRMAR)

| ID | Proposta | Justificativa | Se alterada |
| --- | --- | --- | --- |
| D-005 | Após desarme de segurança, carga permanece **OFF** até novo ON explícito. | Fail-safe; evita reaquecimento automático inesperado. | Se "retomar estado anterior", remover `FR-SAF-004`. |
| D-006 | Recuperação automática de leituras quando o sensor volta (sem reset). | Menor intervenção do operador. | `FR-SNS-005` |
| D-007 | Controle via `POST /control` (`on=1\|0`), resposta com aceite/rejeição. | Semântica clara; não expõe estado em URL de GET. | `FR-API-002` |
| D-008 | Pool DHCP `192.168.4.2–192.168.4.254`. | Cobre clientes; IP do nó é `.1`. | `FR-NET-004` |
| D-009 | Medição de carga idle em janela de **1 s**. | Alinhada à amostragem; métrica estável. | `FR-MET-001` |

## Perguntas bloqueadoras

- **B-001** Clock da CPU: 80 MHz (padrão NodeMCU) vs 160 MHz. Não altera o comportamento funcional; afeta margem de tempo. Confirmar antes do build final.
- **B-002** Driver elétrico do buzzer e da resistência (GPIO direto vs MOSFET/transistor). Necessário para validação HIL; não bloqueia desenvolvimento.
- **B-003** Disponibilidade de bancada para validação física (buzzer, resistência, uptime 24 h).

## Lacunas da descrição (interpretadas)

- A descrição não define o **nível PWM** para "ON" → resolvido (D-001).
- A descrição não define **comportamento do buzzer em falha de sensor** (só fala em "notificação" ao dashboard) → resolvido (D-002).
- A descrição não define a **janela do gráfico de tendência** → resolvido (D-003).
- A descrição não define **formato do endpoint de controle** (só o `/json`) → proposto (D-007).
- A descrição não define **o que ocorre após o desarme** → proposto (D-005).

## Fatos de hardware já validados (repo memory)

- DS18B20 em **D2/GPIO 4** (ROM `28 FF E2 03 B4 16 05 C2`); leituras `-127.0` ocasionais = contato instável.
- Varredura OneWire deve ser one-shot no boot (não periódica no loop).
