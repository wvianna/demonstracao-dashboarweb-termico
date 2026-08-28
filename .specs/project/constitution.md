# Constituição do Projeto — Sistema de Monitoramento e Controle Térmico ESP8266

> Princípios estáveis do projeto. Requisitos específicos de uma feature pertencem a `features/<recurso>/spec.md`.
> Fonte de intenção: `docs/descricao.txt`. Valores adaptados a partir do modelo em `.github/skills/sdd-embarcado/references/constitution.md`.

## Identidade do alvo

- Produto/sistema: Sistema de Monitoramento e Controle Térmico de baixo custo (dashboard web + firmware ESP8266).
- MCU e variante: Espressif ESP8266EX (SoC), núcleo LX106, 32-bit RISC @ 80 MHz (A CONFIRMAR 160 MHz).
- Placa/revisão: NodeMCU v2 (base Amica/DevKit, módulo ESP-12E) — revisão física `A CONFIRMAR`.
- Toolchain/SDK/RTOS: PlatformIO Core + Framework Arduino (`esp8266` core) — versões fixadas no `platformio.ini`; sem RTOS (loop principal + agendamento por `millis()`).
- Clock e alimentação: 3.3 V nativo, alimentação via USB/5V regulada na placa; GPIOs somente em 3.3 V (sem níveis 5 V).
- Ambientes de validação disponíveis: HOST (testes de lógica/estados), BUILD (compilação para o alvo), BANCADA/HIL (NodeMCU + DS18B20 já disponível; buzzer e resistência a confirmar).

## Princípios obrigatórios

### 1. Segurança e estado seguro

- Em falha de sensor, leitura ≥ 80 °C, falha de comunicação ou software, o sistema assume **estado seguro: carga desligada (PWM = 0)**.
- A carga (resistência de aquecimento) possui **intertravamento**: só pode ser acionada se o sensor foi detectado e a temperatura < 80 °C.
- Nenhuma alteração pode contornar o limite térmico de 80 °C, o desligamento por falha ou o intertravamento sem decisão registrada em `STATE.md`.
- Após desarme por segurança, a carga permanece OFF até novo comando ON explícito (comportamento fail-safe).

### 2. Determinismo e concorrência

- **Proibido** usar `delay()` bloqueante e interrupções de hardware/timers para amostragem, alarme ou controle: agendamento **não-bloqueante** por `millis()` no loop principal.
- Contexto de execução único (loop principal) → sem locks/ISR; a única concorrência é com o stack do Wi-Fi (cooperativo).
- O caminho de tempo real (amostragem térmica) declara: período 1 s, tolerância ±150 ms, sem deadline de ISR (não há ISR).
- Alocação dinâmica é **proibida** no caminho de tempo real; buffers (JSON, leitura) pré-alocados/estáticos.

### 3. Recursos limitados

- Limite de flash: **4 MB** (plataforma); meta de binário ≤ 1.5 MB; margem mínima esperada: ≥ 40% livre.
- Limite de RAM: heap livre em operação **≥ 10 KB** (margem mínima), medido via `/json` e monitor serial.
- Uso de energia: sem requisito explícito de baixo consumo (rede sempre ativa); método de medição: `A DEFINIR`.
- Persistência: **nenhuma** (sem EEPROM/SPIFFS); todo estado é volátil.

### 4. Interfaces de hardware e comunicação

- Pinagem, níveis, unidade e escala documentados em `codebase/TARGET.md` antes da implementação; alterações exigem revisão de compatibilidade.
- Protocolo OneWire (DS18B20): scan de ROM no boot, resolução 9–12 bit, leitura em graus Celsius.
- Comunicação web: HTTP/1.1 na porta 80; payloads JSON (`/json`); polling AJAX sem sessão/cookies.

### 5. Qualidade e rastreabilidade

- Todo requisito funcional recebe ID `FR-###`; todo requisito não funcional recebe `NFR-###`.
- Cada requisito alterado possui teste ou evidência; pendências registram risco residual e responsável.
- Build reproduzível com versões registradas no `platformio.ini`; warnings tratados (política: zero warnings no alvo).
- Código gerado por ferramenta não substitui revisão de contrato, segurança e comportamento no alvo.

### 6. Diagnóstico e recuperação

- Logs/telemetria: mensagens curtas via serial (115200 baud) e estado via `/json`; taxa limitada (1/s de leitura; eventos sob demanda).
- Watchdog do ESP8266 deve permanecer alimentado (sem `delay()`); reset inesperado registra motivo no boot.
- Política de recuperação após falha: **permanecer seguro** (carga OFF) e **retomar monitoramento automaticamente** quando o sensor voltar; buzzer e alertas refletem a condição vigente.

### 7. Processo de mudança

- A especificação é atualizada quando o comportamento aprovado muda; não se corrige apenas o código deixando o contrato obsoleto.
- Decisões que alteram risco, arquitetura, timing, energia, memória ou compatibilidade são registradas em `STATE.md` ou no design da feature.
- Uma tarefa deve ser pequena o bastante para revisão e verificação isoladas (ver `features/controle-termico/tasks.md`).
- Não se adicionam abstrações, dependências ou camadas sem benefício verificável no alvo.

## Gates padrão

- [ ] Requisitos e critérios são observáveis e possuem IDs.
- [ ] Alvo, versão de toolchain e dependências foram confirmados.
- [ ] Caminhos de erro, reset, watchdog e estado seguro foram considerados.
- [ ] Timing, concorrência, RAM, flash e energia foram avaliados quando aplicáveis.
- [ ] Testes foram executados no nível declarado: `HOST`, `SIMULADOR`, `BANCADA` ou `HIL`.
- [ ] O resultado e as limitações estão registrados.
