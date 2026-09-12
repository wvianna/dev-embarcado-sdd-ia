# ROADMAP — Sistema de Monitoramento e Controle Térmico (NodeMCU v2 / ESP8266)

| Campo | Valor |
|---|---|
| Data | 2026-09-12 |
| Fase | Planejamento do produto (pré-spec) |
| Classificação do fluxo | GRANDE — exige `spec.md` + `design.md` + `tasks.md` antes da implementação |
| Fonte normativa | `docs/descricao.txt` (prevalece em conflito); complemento: `docs/estudo-de-caso.txt` |
| Regras permanentes | `AGENTS.md` (ordem de leitura obrigatória, restrições de hardware, convenções) |
| Natureza da tarefa | **Reimplementação completa** a partir das fontes; `firmware/` e `.specs/` não existem na árvore de trabalho — código anterior não será restaurado |
| Níveis de evidência aplicáveis | `HOST` (lógica pura) e `BANCADA` (hardware real); `SIMULADOR` e `HIL` não se aplicam neste projeto |

Este documento define visão, escopo, objetivos, riscos, critérios de sucesso e marcos. Os requisitos observáveis (`FR-###`/`NFR-###`) e seus critérios de aceitação serão detalhados no `spec.md` da feature, seguindo `.github/skills/sdd-embarcado/SKILL.md`.

## 1. Visão e escopo

### 1.1 Visão

Firmware para NodeMCU v2 (ESP8266MOD) que monitora temperatura via DS18B20, controla uma resistência de aquecimento por PWM (0–1023) e opera como AP Wi-Fi autônomo com dashboard web e endpoint JSON — tudo em loop único, não bloqueante, sem persistência, com política de segurança térmica determinística (corte em ≥ 80,0 °C, latch até rearme manual e bloqueio de carga sem leitura válida).

### 1.2 Dentro do escopo

- Projeto PlatformIO em `firmware/` com lógica pura portável em `firmware/lib/thermal_logic/` (sem `Arduino.h`) e código de plataforma em `firmware/src/`.
- Máquina de estados térmica, política de segurança (corte, latch, rearme, sensor ausente/inválido), agendamento não bloqueante e serialização JSON.
- Stack de rede: AP aberto com SSID derivado do MAC, IP fixo `192.168.4.1/24`, DHCP e HTTP na porta 80.
- Dashboard web responsivo (tema claro, gauge, gráfico de tendência com escala fixa, botão ON/OFF, tooltips, métricas de saúde do MCU) e endpoint `/json`, conforme `docs/descricao.txt` §5.
- Testes Unity em `firmware/test/` (env `native`) e validação de bancada com hardware real.
- Documentação do fluxo: `README.md`, `STATUS.md`, `HANDOFF.md`, evidências de teste e rastreabilidade.

### 1.3 Fora do escopo

- Persistência de qualquer tipo (EEPROM/SPIFFS) — restrição permanente em `AGENTS.md` §5.
- OTA, HTTPS/TLS, autenticação e fechamento do AP (AP aberto é decisão de produto registrada).
- Acesso à internet e recursos por CDN (o AP não tem internet): HTML/CSS/JS embutidos em flash.
- Controle PID ou qualquer controle avançado — citado em `docs/descricao.txt` apenas como possibilidade futura, não como requisito.
- Múltiplos sensores, RTOS, deep sleep, telemetria externa.
- Restauração de código ou documentação antigos do histórico git (reimplementação a partir das fontes).
- Alteração de `docs/descricao.txt`, `docs/estudo-de-caso.txt` e `AGENTS.md`.

## 2. Objetivos mensuráveis

| ID | Objetivo | Métrica / gate |
|---|---|---|
| O1 | Lógica pura coberta por testes HOST | `pio test -d firmware -e native` com **100% dos testes aprovados**, incluindo fronteira 79,9/80,0 °C, sensor ausente, falha de leitura, latch/rearme e agendamento |
| O2 | Firmware compilando para o alvo | `pio run -d firmware` sem erros; sketch e RAM medidos e registrados; flash dentro do limite de 4 MB da placa (`docs/descricao.txt` §6) |
| O3 | Amostragem dentro da tolerância, inclusive sob carga HTTP | Período medido de 1200 ms com erro absoluto ≤ 150 ms (DEC-07), em bancada, com cliente HTTP em polling contínuo |
| O4 | Corte de segurança e latch verificados na fronteira | Em bancada: leitura 79,9 °C mantém aquecimento; leitura 80,0 °C → PWM 0 na mesma leitura; buzzer no ciclo 150 ms ON / 2000 ms OFF enquanto persistir; rearme somente manual |
| O5 | Interface web operacional via AP | Dashboard e `/json` acessíveis de cliente Wi-Fi real, em sessões de teste contínuas, sem reset do MCU |
| O6 | Rastreabilidade completa dos requisitos críticos | Toda ramificação de segurança e todo limite têm `FR-###`/`NFR-###`, teste associado e evidência registrada |

## 3. Premissas e restrições

### 3.1 Premissas

- Hardware físico disponível para bancada: placa em `/dev/ttyUSB0` e host com Wi-Fi (fornecido pelo contexto da tarefa).
- `docs/descricao.txt` é a fonte normativa do produto; em conflito com `docs/estudo-de-caso.txt`, prevalece a primeira.
- O firmware não depende de infraestrutura de rede pré-existente: o próprio ESP8266 é o AP.
- Plataforma e dependências externas disponíveis (ver seção 7); versões exatas a fixar no `platformio.ini` (QA-5).

### 3.2 Restrições

- Pinagem fixa, níveis lógicos, limites elétricos e proibições (sem ISR de hardware, sem timers de amostragem, sem `delay()` bloqueante, sem persistência): ver `AGENTS.md` §5 — restrição permanente, não renegociável neste ROADMAP.
- GPIO16 (buzzer) não suporta PWM nem interrupções: usar apenas `digitalRead`/`digitalWrite`.
- PWM da resistência em 10 bits (`analogWriteRange(1023)`), 0 = desligado, 1023 = potência máxima.
- Conversão DS18B20 assíncrona (`setWaitForConversion(false)`), leitura ao final do ciclo e re-disparo imediato.
- AP aberto (sem senha), IP fixo `192.168.4.1`, DHCP ativo, HTTP porta 80.
- Documentação em pt-BR; identificadores de código em inglês; HTML sem CDN externo.
- Orçamento de memória: sem `std::string`/`std::vector` no hot path; buffers estáticos; strings de UI em PROGMEM (`AGENTS.md` §6).

## 4. Riscos e mitigação

| ID | Risco | Mitigação | Evidência esperada |
|---|---|---|---|
| R1 | **Segurança térmica/latch**: falha de leitura ou temperatura ≥ 80,0 °C sem corte imediato; latch que não se mantém ou rearma sem ação manual | Política de segurança como lógica pura determinística, com a condição de corte separada do controle normal; sensor ausente ou leitura inválida bloqueia a carga; testes HOST de fronteira e injeção em bancada; latch só sai por rearme manual (QA-1) | Testes HOST na fronteira 79,9/80,0 e de falha; sessão de bancada com corte, latch e rearme observados |
| R2 | **Recursos de RAM/Flash do ESP8266**: estouro de heap por buffers dinâmicos ou HTML pesado; fragmentação em execução longa | Buffers estáticos de tamanho definido; HTML/CSS/JS em PROGMEM; nenhuma alocação no loop; medir sketch/RAM no build e monitorar uso de RAM no dashboard durante a bancada | Footprint registrado em M2; sessão prolongada em bancada sem reset (M3) |
| R3 | **Timing 1,2 s ±150 ms sob carga HTTP** (DEC-07): jitter ou atraso no ciclo de amostragem quando o servidor atende requisições | Agendamento por `millis()` desacoplado do atendimento HTTP; conversão DS18B20 assíncrona; medição do período em bancada com polling contínuo no dashboard/`/json` | Série de medições de período em bancada dentro da tolerância (M3) |
| R4 | **Validação física obrigatória**: itens não prováveis em HOST (buzzer em GPIO16, PWM real da carga, OneWire físico, AP/DHCP/HTTP, comportamento de boot/watchdog) | Checklist de bancada dedicado; **proibido declarar `BANCADA` sem execução em hardware real**; itens não cobertos ficam registrados como risco residual em `HANDOFF.md` | Checklist de bancada preenchido com logs e medições em `docs/05-testing/` |
| R5 | **Reset/watchdog por bloqueio acidental** durante operação longa ou atendimento HTTP | Proibição de `delay()` e de laços bloqueantes; testes de sessão contínua em bancada; verificação de boot e recuperação | Sessão de bancada contínua sem reset espontâneo (M3) |
| R6 | **Deriva de requisitos na reimplementação**: perder comportamentos exigidos pelas fontes ou reintroduzir decisões do código antigo | Rastreabilidade contínua (FR/NFR → teste → evidência) e revisão contra `docs/descricao.txt` antes de cada marco | Matriz de rastreabilidade atualizada (M1–M4) |

## 5. Critérios de sucesso

- [ ] `pio test -d firmware -e native` verde (gate mínimo de entrega).
- [ ] `pio run -d firmware` limpo, com sketch/RAM medidos e registrados.
- [ ] Bancada: AP associável, DHCP entregando endereço, dashboard em `http://192.168.4.1` e `/json` com payload válido.
- [ ] Bancada: fronteira 79,9 °C (sem corte) e 80,0 °C (corte imediato), buzzer 150 ms/2000 ms, latch exigindo rearme manual.
- [ ] Bancada: período de amostragem dentro de 1200 ms ± 150 ms, inclusive sob carga HTTP.
- [ ] Evidências registradas em `docs/05-testing/` (build, resultados de teste, logs seriais, medições) e rastreabilidade atualizada.
- [ ] Gate de verificação aplicado conforme `docs/agentic/DEFINITION-OF-DONE.md`; riscos residuais e pendências de validação física explícitos em `STATUS.md`/`HANDOFF.md`.

## 6. Marcos

### M1 — Lógica pura e testes HOST

- [ ] FSM térmica, política de segurança (corte/latch/rearme), bloqueio por sensor ausente/inválido e agendamento por `millis()` em `firmware/lib/thermal_logic/` (sem `Arduino.h`).
- [ ] Serialização do payload JSON como lógica pura.
- [ ] Testes Unity em `firmware/test/` cobrindo fronteira 79,9/80,0 °C, falha de leitura, latch/rearme e tolerância de agendamento.
- **Saída:** todos os testes PASS; nenhuma ramificação de segurança sem teste correspondente.

### M2 — Build alvo com footprint medido

- [ ] `pio run -d firmware` no env do alvo, sem erros.
- [ ] Medição de sketch e RAM registrada, com inspeção de uso de memória (PROGMEM, buffers estáticos, ausência de `String` no loop).
- **Saída:** SUCCESS + footprint registrado em `docs/05-testing/`.

### M3 — Gravação e validação de bancada com hardware real

- [ ] Gravação em `/dev/ttyUSB0` e acompanhamento por monitor serial (115200 baud).
- [ ] AP/DHCP/HTTP validados com cliente Wi-Fi real (dashboard e `/json`).
- [ ] Fronteira de temperatura verificada (injeção e/ou ensaio de bancada, conforme QA-4): corte, buzzer, latch e rearme.
- [ ] Período de amostragem medido com carga HTTP; sessão contínua sem reset.
- [ ] Evidências registradas (logs, medições, capturas quando aplicável).
- **Saída:** checklist de bancada concluído no nível `BANCADA`; o que não puder ser verificado fisicamente vira risco residual explícito.

### M4 — Fechamento

- [ ] Gate de verificação (`DEFINITION-OF-DONE.md`) aplicado a todas as tarefas.
- [ ] Matriz de rastreabilidade `FR/NFR → teste → evidência → CA-###` completa.
- [ ] `README.md`, `STATUS.md` e `HANDOFF.md` atualizados, com decisões, pendências e riscos residuais.
- **Saída:** critérios de sucesso da seção 5 verificados ou explicitamente bloqueados com justificativa.

## 7. Dependências externas

- **PlatformIO Core** (CLI `pio`) — build, upload, monitor e testes.
- **Plataforma `espressif8266` + framework Arduino Core para ESP8266** — versões a fixar no `platformio.ini` (QA-5).
- **Toolchain `xtensa-lx106`** — gerenciado pelo PlatformIO.
- **Toolchain C/C++ nativo do host** — para o env `native` (testes Unity).
- **Bibliotecas `OneWire` e `DallasTemperature`** — versões a fixar (QA-5).
- **Hardware de bancada**: NodeMCU v2, DS18B20, buzzer ativo, resistência/carga com acionamento adequado, fonte, host com Wi-Fi e porta serial `/dev/ttyUSB0`.
- **Meio de referência térmica** (fonte de calor ou método de injeção no env `bancada`) — estratégia a definir em QA-4; se não houver meio de referência calibrado, a verificação da fronteira de 80 °C fica limitada a injeção + HOST, com registro explícito da limitação.

## 8. Questões abertas

| ID | Questão | Impacto | Onde será decidida |
|---|---|---|---|
| QA-1 | Qual é o mecanismo de rearme manual do latch (botão no dashboard, comando HTTP, ação física)? | Interface e comportamento de segurança observável | `spec.md` (FR) |
| QA-2 | Sem persistência, o latch é volátil: qual o estado esperado após reset/watchdog, e qual o estado da carga entre o boot e a primeira leitura válida? | Segurança térmica em partida e recuperação | `spec.md` + `design.md` |
| QA-3 | Qual o contrato do endpoint `/json` (nomes de campos, unidades, como representar falha do sensor)? O `docs/estudo-de-caso.txt` exige que falha do DS18B20 apareça explicitamente no dashboard. | Integração dashboard ↔ firmware | `spec.md` (FR) |
| QA-4 | Como a bancada atingirá a fronteira 79,9/80,0 °C (injeção no env `bancada`, ensaio térmico real ou ambos) e com que segurança elétrica/térmica? | Validação física do requisito mais crítico | `design.md` + `tasks.md` |
| QA-5 | Quais versões exatas de plataforma, framework e bibliotecas serão fixadas? | Reprodutibilidade do build | `design.md` / `platformio.ini` |

## Referências

- `AGENTS.md` — regras permanentes, restrições de hardware e comandos.
- `docs/descricao.txt` — fonte normativa do produto (não editar).
- `docs/estudo-de-caso.txt` — complemento ao produto (não editar).
- `.github/skills/sdd-embarcado/SKILL.md` — fluxo SDD adaptativo e artefatos exigidos.
- `docs/agentic/DEFINITION-OF-DONE.md`, `docs/agentic/TRACEABILITY.md`, `docs/agentic/WORKFLOW.md` — critérios de conclusão, rastreabilidade e fluxo entre agentes.
