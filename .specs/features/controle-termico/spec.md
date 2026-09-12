# spec.md — Feature `controle-termico`

| Campo | Valor |
|---|---|
| Feature | `controle-termico` (recurso novo — reimplementação completa do firmware) |
| Data | 2026-09-12 |
| Agente | 02-requirements-engineer |
| Classificação | Fluxo GRANDE (`.specs/project/ROADMAP.md`) |
| Fontes | F1 = `docs/descricao.txt` (normativa; prevalece em conflito); F2 = `docs/estudo-de-caso.txt` (complemento) |
| Documentos correlatos | `.specs/project/constitution.md`, `.specs/project/ROADMAP.md`, `.specs/codebase/CODEBASE.md`, `AGENTS.md` |
| Próximos artefatos | `design.md` (03) e `tasks.md` (04) |

## 1. Objetivo e fora de escopo

**Objetivo.** Especificar o comportamento observável do firmware de monitoramento e controle térmico em NodeMCU v2 (ESP8266MOD): varredura e leitura do DS18B20, controle da resistência por PWM (0–1023), política de segurança (corte em ≥ 80,0 °C com latch e rearme manual, bloqueio em falha de sensor), rede AP própria com dashboard web e telemetria JSON, e console de diagnóstico de bancada.

**Escopo macro** (dentro/fora do produto) está definido em ROADMAP §1.2/§1.3 — não é duplicado aqui. Esta spec detalha os requisitos observáveis das áreas: boot/varredura, amostragem, controle da carga, segurança térmica, falha de sensor, boot/reset/watchdog, rede, dashboard, telemetria e console de bancada.

**Fora de escopo desta spec** (pertencem aos próximos artefatos ou ao ROADMAP): contrato HTTP detalhado, nomes/unidades de campos JSON, arquitetura de módulos e ADRs → `design.md`; decomposição em tarefas → `tasks.md`; itens fora do produto (persistência, OTA, TLS, autenticação, PID, múltiplos sensores, RTOS) → ROADMAP §1.3.

**Invariantes de segurança** (não renegociáveis sem decisão registrada): T ≥ 80,0 °C ou leitura inválida ⇒ PWM 0; latch só sai por rearme manual com leitura válida < 80,0 °C; a carga nunca é acionada sem leitura válida e comando explícito; nenhum caminho automático religa a carga. Codificadas em FR-009…FR-015.

## 2. Atores, estados e eventos

### 2.1 Atores

| Ator | Descrição | Interação |
|---|---|---|
| Operador | Usuário local do sistema | Dashboard `/` (visualização, ON/OFF, rearme) e `/json` |
| Técnico de bancada | Responsável pela validação física | Console serial 115200 do env `bancada` (injeção, leitura de estado, comandos) |
| Cliente de telemetria | Consumidor do endpoint JSON | Polling HTTP em `/json` (a própria dashboard via AJAX) |
| Ambiente físico | DS18B20 (temperatura), resistência (carga), buzzer (sinalização) | GPIO4 (OneWire), GPIO5 (PWM), GPIO16 (digital) |

### 2.2 Estados do sistema

| Estado | Condição | Comportamento da carga | Interface |
|---|---|---|---|
| INICIALIZANDO | `setup` em andamento (varredura OneWire) | Desligada (PWM 0); comandos recusados | Estado inicial exposto |
| SEM_SENSOR | Nenhum DS18B20 detectado; re-varredura a cada 5 s | Bloqueada (PWM 0) | Estado "sensor ausente" explícito |
| FALHA_LEITURA | Sensor presente, mas leitura inválida/erro de comunicação | Bloqueada (PWM 0) | Alerta crítico explícito |
| OPERACAO_NORMAL | Leitura válida; latch inativo | Segue comando ON/OFF do operador | Temperatura e estado da carga |
| ALARME_LATCHED | Leitura válida ≥ 80,0 °C (latch ativo) | Bloqueada (PWM 0) até rearme manual | Alarme, latch ativo e ação de rearme |

Observações de composição: o latch (`ALARME_LATCHED`) é ortogonal à condição do sensor — uma falha de leitura durante o latch mantém o latch ativo; a perda de validade não libera bloqueio algum. Após rearme aceito ou após retomada de leitura válida em `FALHA_LEITURA`, a carga permanece desligada até novo comando ON (DEC-01, DEC-04).

### 2.3 Eventos

| Evento | Efeito no sistema |
|---|---|
| Boot / reset / watchdog / brownout | Estado seguro: carga desligada, latch volátil (limpo), bloqueio até primeira leitura válida (DEC-02) |
| Varredura OneWire sem sensor | Estado SEM_SENSOR; repetir varredura a cada 5 s |
| Sensor detectado em runtime | Sai de SEM_SENSOR; leituras passam a valer após primeira leitura válida |
| Sensor perdido / erro de leitura em operação | PWM 0 imediato; alerta crítico; falha representada explicitamente; sem buzzer |
| Leitura válida restabelecida (sem latch) | Bloqueio removido; carga permanece desligada até novo comando ON (DEC-04) |
| Comando ON/OFF | Efetivado se permitido; recusado com feedback explícito se bloqueado (latch, sensor ausente, leitura inválida) |
| Leitura válida ≥ 80,0 °C | PWM 0 imediato + latch + buzzer em ciclo 150 ms/2000 ms |
| Temperatura cai abaixo de 80,0 °C com latch ativo | Buzzer cessa; latch e bloqueio permanecem |
| Rearme manual solicitado | Aceito somente com leitura válida < 80,0 °C; caso contrário, recusa explícita |
| Requisição HTTP (`/`, `/json`, comandos) | Atendida em qualquer estado; comandos respeitam a política de bloqueio |
| Comandos do console (env `bancada`) | Injeção de temperatura/falha, leitura de estado, comandos de carga e rearme |

### 2.4 Casos de uso

- **UC-01 — Operar a carga (operador):** ligar/desligar a resistência e observar o estado (FR-007…FR-009, FR-024).
- **UC-02 — Monitorar (operador):** acompanhar temperatura, tendência, saúde do MCU e telemetria (FR-021…FR-029).
- **UC-03 — Responder ao alarme (operador):** corte automático em ≥ 80,0 °C, sinalização sonora/visual, diagnóstico de falha de sensor e rearme manual (FR-010…FR-015, FR-026).
- **UC-04 — Diagnosticar em bancada (técnico):** injetar temperatura/falha, ler estado e comandar carga/rearme para gerar evidência (FR-030, CA-024).

## 3. Interfaces de hardware

| Interface | Pino / instância | Protocolo / modo | Faixa e resolução | Temporização | Falha / fora da faixa |
|---|---|---|---|---|---|
| DS18B20 | D2 / GPIO4 | OneWire (barramento de 1 fio) | Temperatura em °C (resolução definida no design) | Amostragem 1200 ms ± 150 ms (FR-004; DEC-07) | Ausência/erro de comunicação ⇒ leitura inválida ⇒ carga bloqueada + alerta crítico (FR-014) |
| Resistência (carga) | D1 / GPIO5 | PWM por software, 10 bits (`analogWriteRange(1023)`) | 0 a 1023; 0 = desligado, 1023 = potência máxima | Aplicação imediata no corte (FR-010) | Bloqueio de segurança ⇒ PWM 0 (sempre) |
| Buzzer ativo | D0 / GPIO16 | Digital on/off (sem PWM e sem interrupção) | 3,3 V | Alarme: 150 ms ON / 2000 ms OFF (FR-013) | Não opera em falha de sensor (FR-013) |
| Wi-Fi / HTTP | AP interno do ESP8266 | AP aberto; HTTP na porta 80 | SSID `ESP8266_XXXXXX`; 192.168.4.1/24; DHCP ativo | Resposta HTTP ≤ 500 ms (NFR-004, DEC-06) | AP autônomo; sem internet nem CDN (ROADMAP §1.3) |

Premissa física em aberto (constitution §4): pull-up do barramento OneWire **A CONFIRMAR** no hardware (típico 4,7 kΩ); não é controlado pelo firmware.

## 4. Requisitos funcionais

### 4.1 Boot, varredura e presença do sensor

| ID | Requisito | Prioridade | Critérios |
|---|---|---|---|
| FR-001 | No `setup`, executar a varredura OneWire do barramento e identificar o endereço único (ROM) do DS18B20; registrar o resultado (detectado/ausente e endereço) no log serial e no estado exposto pela interface. O endereço identificado passa a ser a referência das conversões e leituras subsequentes (vínculo de ROM — DEC-07); a re-varredura de 5 s (FR-003) refaz a enumeração do barramento e atualiza o vínculo se o sensor for substituído. | Crítica | CA-001, CA-002, CA-017 |
| FR-002 | Sem sensor detectado no boot, a carga permanece bloqueada (PWM 0) e nenhum comando ON é efetivado; o estado "sensor ausente" é explícito no dashboard e em `/json`. | Crítica | CA-002, CA-007, CA-017 |
| FR-003 | Enquanto não houver sensor detectado, repetir a varredura OneWire a cada 5 s, de forma não bloqueante; se o sensor passar a responder em runtime, o sistema sai do estado SEM_SENSOR e volta a produzir leituras. | Crítica | CA-003 |

### 4.2 Amostragem térmica

| ID | Requisito | Prioridade | Critérios |
|---|---|---|---|
| FR-004 | Amostrar a temperatura em período nominal de 1200 ms com tolerância de ± 150 ms (DEC-07), de forma não bloqueante, integrada ao loop principal via `millis()`; o agendamento não pode depender do atendimento HTTP. | Crítica | CA-004, CA-021 |
| FR-005 | Conversão DS18B20 assíncrona: `setWaitForConversion(false)`; nenhuma espera ativa pela conversão; a conversão é disparada para a ROM vinculada (`requestTemperaturesByAddress`) e o resultado é lido ao final do ciclo pela mesma ROM (`getTempC`), re-disparando a próxima conversão imediatamente (DEC-07). | Crítica | CA-004, CA-005 |
| FR-006 | Classificar cada leitura como válida ou inválida: sensor presente e resposta sem erro de comunicação ⇒ válida (qualquer temperatura, inclusive ≥ 80,0 °C, que é condição de segurança e não invalidez); erro, timeout ou ausência de resposta ⇒ inválida. Somente leitura válida alimenta controle, alarme, histórico e apresentação. | Crítica | CA-015, CA-017 |

### 4.3 Controle da carga

| ID | Requisito | Prioridade | Critérios |
|---|---|---|---|
| FR-007 | Disponibilizar ao operador o botão ON/OFF da resistência no dashboard, com comando equivalente por HTTP (contrato das rotas definido no design); o comando só é efetivado quando o sistema permite acionamento. | Alta | CA-006, CA-007 |
| FR-008 | Efetivar ON ⇒ PWM 1023 (potência máxima); OFF ⇒ PWM 0 (desligado); PWM em 10 bits com `analogWriteRange(1023)`; o estado aplicado é observável na interface. | Crítica | CA-006 |
| FR-009 | Recusar comando ON com feedback explícito ao operador quando houver bloqueio (latch ativo, sensor ausente ou leitura inválida), sem alterar o PWM (permanece 0). | Crítica | CA-007 |

### 4.4 Segurança térmica

| ID | Requisito | Prioridade | Critérios |
|---|---|---|---|
| FR-010 | Leitura válida ≥ 80,0 °C ⇒ PWM 0 imediato, na mesma avaliação (independentemente do estado da carga), e ativação do latch de segurança. | Crítica | CA-008, CA-009 |
| FR-011 | Com latch ativo, a carga permanece bloqueada mesmo que a temperatura caia abaixo de 80,0 °C e com leituras válidas; nenhum caminho de código libera o latch automaticamente. | Crítica | CA-010 |
| FR-012 | O latch só sai por rearme manual: botão no dashboard e comando equivalente no console de bancada (DEC-01). Aceito apenas com leitura válida < 80,0 °C no momento do pedido; caso contrário, recusa explícita e latch mantido. Após rearme aceito, a carga permanece desligada até novo comando ON. | Crítica | CA-011, CA-012 |
| FR-013 | Buzzer em ciclo de 150 ms ligado / 2000 ms desligado apenas enquanto a condição T ≥ 80,0 °C persistir (com leituras válidas); cessar quando a condição deixar de existir; não operar buzzer em falha de sensor/leitura inválida. | Crítica | CA-013, CA-014 |

### 4.5 Falha de sensor e leitura inválida

| ID | Requisito | Prioridade | Critérios |
|---|---|---|---|
| FR-014 | Falha de leitura/sensor em operação ⇒ PWM 0 imediato + alerta crítico imediato no dashboard + representação explícita do estado de falha (ausência/validade) no dashboard e em `/json`; a falha nunca é apresentada como temperatura numérica plausível. | Crítica | CA-015, CA-016, CA-017 |
| FR-015 | Retomada após falha de leitura (sem latch): o bloqueio persiste enquanto a leitura for inválida e é removido quando uma leitura válida for obtida; a carga não é religada automaticamente — permanece desligada até novo comando ON (DEC-04). | Crítica | CA-016 |

### 4.6 Boot, reset e watchdog

| ID | Requisito | Prioridade | Critérios |
|---|---|---|---|
| FR-016 | Após boot/reset (incluindo watchdog e brownout): carga inicia desligada (PWM 0); latch é volátil (sem persistência — sem EEPROM/SPIFFS); acionamento bloqueado até a primeira leitura válida (DEC-02). | Crítica | CA-001, CA-018 |
| FR-017 | Registrar no log serial o motivo do boot/reset quando as APIs do core permitirem, mantendo o estado seguro (DEC-02). | Média | CA-018 |

### 4.7 Rede e servidor HTTP

| ID | Requisito | Prioridade | Critérios |
|---|---|---|---|
| FR-018 | Operar como Access Point aberto (sem criptografia), com SSID derivado do MAC no formato `ESP8266_<3 últimos bytes em hexadecimal maiúsculo>` (ex.: `ESP8266_1A2B3C`) — DEC-05. | Alta | CA-019 |
| FR-019 | IP fixo 192.168.4.1/24, gateway 192.168.4.1, máscara 255.255.255.0; servidor DHCP ativo para clientes na sub-rede 192.168.4.0/24; servidor HTTP na porta 80. | Alta | CA-019 |
| FR-020 | O servidor HTTP permanece operacional de forma contínua: dashboard `/` e `/json` respondem durante alarme (T ≥ 80,0 °C), latch, falha de sensor e comandos de carga/rearme, sem reset do MCU. | Crítica | CA-021 |

### 4.8 Dashboard web (`/`)

| ID | Requisito | Prioridade | Critérios |
|---|---|---|---|
| FR-021 | Dashboard em `/`, em pt-BR, tema claro, responsivo e sem rolagem em desktop (referência 1366×768 — DEC-06). | Alta | CA-022 |
| FR-022 | Exibir a temperatura atual por valor numérico em destaque e por gauge. | Alta | CA-022 |
| FR-023 | Exibir gráfico de tendência com histórico, escala fixa de 20 °C a 90 °C (não auto-ajustável), escala numérica do lado esquerdo e grade de fundo cinza claro. | Alta | CA-022 |
| FR-024 | Exibir botão ON/OFF e indicação do estado da resistência por cor (verde = ligada; vermelho = desligada), com feedback visível da recusa de comando. | Alta | CA-007, CA-022 |
| FR-025 | Exibir no cabeçalho as métricas de saúde do MCU: carga, idle, uso de RAM e uso de flash. | Alta | CA-020, CA-022 |
| FR-026 | Exibir alertas visíveis de alarme (T ≥ 80,0 °C) e de falha de sensor/leitura; indicar explicitamente latch ativo e disponibilizar a ação de rearme. | Crítica | CA-022 |
| FR-027 | Disponibilizar tooltips/hints de ajuda rápida em todos os elementos de controle e de visualização. | Média | CA-022 |

### 4.9 Telemetria (`/json`)

| ID | Requisito | Prioridade | Critérios |
|---|---|---|---|
| FR-028 | `/json` fornece o conjunto obrigatório de informações: temperatura, validade da leitura, presença do sensor, estado do sistema, PWM aplicado, latch, alarme, uptime, RAM livre, uso de flash, carga, idle, histórico para o gráfico e versão do firmware; nomes de campos, unidades, ordem e formato do histórico são fixados no design (DEC-03). | Alta | CA-020 |
| FR-029 | A dashboard consome `/json` por polling assíncrono (AJAX) a cada 2000 ms, sem recarga total da página, refletindo mudanças de estado em ≤ 2,5 s (DEC-06 revisto por DEC-07). | Alta | CA-023 |

### 4.10 Console de diagnóstico de bancada

| ID | Requisito | Prioridade | Critérios |
|---|---|---|---|
| FR-030 | O console serial de diagnóstico é compilado apenas no env `bancada` e permite: injetar temperatura e falha, ler o estado do sistema e emitir comandos de carga (ON/OFF) e rearme; não existe no env de produção (`nodemcuv2`). | Alta | CA-024 |

## 5. Requisitos não funcionais

| ID | Requisito | Prioridade | Critérios |
|---|---|---|---|
| NFR-001 | Estritamente não bloqueante: loop cooperativo com `millis()`; proibidos `delay()`, interrupções de hardware, timers de hardware, espera ativa e persistência (EEPROM/SPIFFS); sem concorrência preemptiva; buffers de owner único (loop principal). | Crítica | CA-026 |
| NFR-002 | Memória: sem alocação dinâmica no loop; sem `String` em acúmulo; sem `std::string`/`std::vector` no alvo; buffers estáticos de tamanho definido; HTML/CSS/JS em PROGMEM. Metas da constituição: sketch ≤ 45% de 1.044.464 B e RAM estática ≤ 50% de 81.920 B; footprint medido e registrado a cada build (flash total da placa: 4 MB — F1 §6). | Crítica | CA-025 |
| NFR-003 | Timing: amostragem de 1200 ms ± 150 ms medida, inclusive sob polling HTTP contínuo (DEC-07); buzzer com fases nominais de 150 ms / 2000 ms verificadas em bancada com tolerância de ± 20% por fase (DEC-06). | Crítica | CA-004, CA-013, CA-021 |
| NFR-004 | Responsividade HTTP: com alarme ativo e polling ≥ 1 Hz, toda requisição `/` e `/json` completa em ≤ 500 ms (bancada); o atendimento HTTP não pode deslocar a amostragem para fora de NFR-003 (DEC-06). | Alta | CA-021 |
| NFR-005 | Reprodutibilidade: build via PlatformIO com plataforma, framework e bibliotecas em versões fixadas em `firmware/platformio.ini` (QA-5 → design); toolchain e ambientes documentados; footprint registrado. | Alta | CA-027 |
| NFR-006 | Rastreabilidade e verificação: todo FR/NFR possui critério e teste/evidência associados; `PASS` exige evidência registrada em `docs/05-testing/`; validação de hardware somente em nível BANCADA; desvios registrados como `SPEC_DEVIATION`. | Alta | CA-028 |

## 6. Critérios de aceitação

Legenda de evidência: **HOST** = testes Unity da lógica pura, inspeção estática e medições de build na máquina de desenvolvimento; **BANCADA** = placa real em `/dev/ttyUSB0` com hardware (nunca substituída por HOST — `AGENTS.md` §7). `HOST + BANCADA` exige ambos para `PASS`.

| ID | Requisitos | Critério (DADO / QUANDO / ENTÃO) | Nível de evidência |
|---|---|---|---|
| CA-001 | FR-001, FR-016 | DADO placa energizada com DS18B20 conectado; QUANDO o `setup` conclui a varredura OneWire; ENTÃO o endereço único é identificado/registrado E a carga inicia desligada (PWM 0) E o acionamento permanece bloqueado até a primeira leitura válida. | HOST + BANCADA |
| CA-002 | FR-001, FR-002 | DADO placa energizada sem DS18B20; QUANDO o `setup` executa a varredura; ENTÃO a carga é inacionável (PWM 0; ON recusado) E o estado "sensor ausente" aparece no dashboard e em `/json` E a varredura é repetida a cada 5 s. | HOST + BANCADA |
| CA-003 | FR-003 | DADO sistema bloqueado por sensor ausente; QUANDO o DS18B20 é conectado ao barramento; ENTÃO o sensor é detectado na próxima re-varredura (≤ 5 s) E leituras válidas passam a alimentar o sistema em até 2 ciclos de amostragem seguintes. | HOST + BANCADA |
| CA-004 | FR-004, FR-005, NFR-003 | DADO operação com leituras válidas e cliente HTTP em polling ≥ 1 Hz; QUANDO 60 intervalos consecutivos de amostragem são registrados; ENTÃO cada intervalo fica em [1050 ms, 1350 ms] (DEC-07). | HOST + BANCADA |
| CA-005 | FR-005 | DADO ciclo de amostragem em execução; QUANDO a leitura é coletada; ENTÃO nenhuma etapa aguarda a conversão de forma bloqueante (resultado lido ao final do ciclo e próxima conversão disparada imediatamente), verificável no log de ciclo. | BANCADA |
| CA-006 | FR-007, FR-008 | DADO leitura válida < 80,0 °C, sensor presente, latch inativo e carga desligada; QUANDO o operador aciona ON; ENTÃO PWM = 1023 e indicador verde; QUANDO aciona OFF; ENTÃO PWM = 0 e indicador vermelho. | HOST + BANCADA |
| CA-007 | FR-002, FR-009, FR-015, FR-024 | DADO qualquer condição de bloqueio (latch ativo, sensor ausente ou leitura inválida); QUANDO o operador aciona ON; ENTÃO o comando é recusado com feedback explícito E o PWM permanece 0. | HOST + BANCADA |
| CA-008 | FR-010 | DADO carga ligada com leitura válida de 79,9 °C; QUANDO a leitura é avaliada; ENTÃO a carga permanece ligada (PWM 1023), sem alarme e sem latch. | HOST + BANCADA |
| CA-009 | FR-010 | DADO carga ligada com leitura válida alcançando 80,0 °C; QUANDO a leitura é avaliada; ENTÃO PWM = 0 na mesma avaliação E latch ativo E alarme sinalizado. | HOST + BANCADA |
| CA-010 | FR-011 | DADO latch ativo; QUANDO a temperatura cai para 50,0 °C com leituras válidas; ENTÃO a carga permanece desligada E o latch permanece ativo E o buzzer cessa E o dashboard indica latch. | HOST + BANCADA |
| CA-011 | FR-012 | DADO latch ativo e leitura válida < 80,0 °C; QUANDO o rearme é acionado (dashboard ou console de bancada); ENTÃO o latch é liberado E a carga permanece desligada até novo comando ON. | HOST + BANCADA |
| CA-012 | FR-012 | DADO latch ativo; QUANDO o rearme é solicitado com leitura inválida/ausente ou com leitura válida ≥ 80,0 °C; ENTÃO o rearme é recusado com feedback explícito E o latch permanece ativo E a carga segue bloqueada. | HOST + BANCADA |
| CA-013 | FR-013, NFR-003 | DADO condição T ≥ 80,0 °C persistente; QUANDO o buzzer é medido/observado por ≥ 5 ciclos; ENTÃO as fases medem aproximadamente 150 ms e 2000 ms dentro de ± 20% E a amostragem continua dentro de CA-004. | HOST + BANCADA |
| CA-014 | FR-013 | DADO falha de sensor/leitura inválida (sem condição ≥ 80,0 °C); QUANDO o estado é avaliado; ENTÃO o buzzer permanece desligado. | HOST + BANCADA |
| CA-015 | FR-006, FR-014 | DADO carga ligada com leituras válidas; QUANDO ocorre falha de leitura/desconexão do sensor; ENTÃO PWM = 0 imediatamente E alerta crítico visível no dashboard E estado de falha explícito em `/json`, sem valor numérico falso. | HOST + BANCADA |
| CA-016 | FR-014, FR-015 | DADO bloqueio por leitura inválida (sem latch); QUANDO leituras válidas retornam; ENTÃO o bloqueio é removido E a carga permanece desligada exigindo novo comando ON E o alerta crítico é encerrado. | HOST + BANCADA |
| CA-017 | FR-001, FR-006, FR-014 | DADO sensor ausente ou leitura inválida; QUANDO a interface é consultada; ENTÃO a ausência/falha é representada explicitamente (estado dedicado, sem apresentar temperatura falsa) no dashboard e em `/json`. | HOST + BANCADA |
| CA-018 | FR-016, FR-017 | DADO sistema em operação; QUANDO ocorre reset forçado (watchdog/brownout ou comando de reboot); ENTÃO após o boot a carga está desligada E o latch foi limpo (volátil) E o acionamento está bloqueado até leitura válida E o motivo do reset é registrado quando disponível. | BANCADA |
| CA-019 | FR-018, FR-019 | DADO firmware em execução; QUANDO um cliente Wi-Fi escaneia e associa; ENTÃO o SSID `ESP8266_XXXXXX` aparece aberto E o cliente recebe IP via DHCP em 192.168.4.0/24 E `http://192.168.4.1/` serve o dashboard. | BANCADA |
| CA-020 | FR-025, FR-028 | DADO cliente conectado; QUANDO requisita `/json`; ENTÃO o payload contém todos os itens obrigatórios com valores coerentes com o estado atual (incluindo latch, alarme, presença/validade do sensor, PWM, métricas de saúde e histórico). | HOST + BANCADA |
| CA-021 | FR-004, FR-020, NFR-003, NFR-004 | DADO alarme ativo (T ≥ 80,0 °C induzida) e polling HTTP ≥ 1 Hz por ≥ 60 s; QUANDO as requisições são atendidas; ENTÃO todas completam em ≤ 500 ms E a amostragem permanece em CA-004 E não ocorre reset. | BANCADA |
| CA-022 | FR-021…FR-027 | DADO dashboard aberto em desktop (1366×768); QUANDO a página é inspecionada; ENTÃO: tema claro; sem rolagem; valor numérico + gauge; gráfico 20–90 °C com escala à esquerda e grade cinza claro; botão ON/OFF com cores de estado; cabeçalho com carga, idle, RAM e flash; alertas de alarme/falha e latch + rearme visíveis; tooltips em controles e elementos gráficos. | BANCADA |
| CA-023 | FR-029 | DADO dashboard aberto; QUANDO o estado muda (ex.: corte por injeção); ENTÃO a página reflete a mudança em ≤ 2,5 s sem recarga manual (polling de 2000 ms — DEC-07). | BANCADA |
| CA-024 | FR-030 | DADO build do env `bancada`; QUANDO comandos de injeção (temperatura/falha), leitura de estado, carga e rearme são enviados; ENTÃO o sistema responde e reflete o efeito; E no build do env `nodemcuv2` esses comandos não existem. | HOST + BANCADA |
| CA-025 | NFR-002 | DADO build do env de produção; QUANDO o footprint é medido; ENTÃO compila sem erros, sketch ≤ 45% de 1.044.464 B e RAM estática ≤ 50% de 81.920 B, sem `String` em acúmulo nem alocação dinâmica no loop (inspeção). | HOST |
| CA-026 | NFR-001 | DADO código do firmware; QUANDO inspecionado e executado em sessão contínua de bancada; ENTÃO não há `delay()`, ISR de hardware, timers de hardware nem persistência E o loop permanece não bloqueante (sem reset por watchdog) durante a sessão. | HOST + BANCADA |
| CA-027 | NFR-005 | DADO checkout limpo e `firmware/platformio.ini` com versões fixadas; QUANDO o build é executado; ENTÃO conclui com sucesso sem resolver versões "latest" E o footprint é registrado. | HOST |
| CA-028 | NFR-006 | DADO o conjunto de requisitos desta spec; QUANDO a verificação final do recurso é executada; ENTÃO cada FR/NFR possui critério, tarefa, teste e evidência registrados (ou pendência justificada) na matriz de rastreabilidade. | HOST |

## 7. Decisões registradas

| ID | Decisão | Resolve | Referências |
|---|---|---|---|
| DEC-01 | Rearme manual do latch = botão no dashboard + comando equivalente no console de bancada; aceito somente com leitura válida < 80,0 °C; recusa explícita caso contrário; nenhum outro caminho libera o latch. | QA-1 | FR-012, CA-011, CA-012 |
| DEC-02 | Após boot/reset/watchdog: carga desligada, latch volátil (sem persistência), bloqueio até a primeira leitura válida; motivo do reset registrado quando a API permitir. | QA-2 | FR-016, FR-017, CA-001, CA-018 |
| DEC-03 | Conjunto obrigatório de informações do `/json` fixado nesta spec; nomes de campos, unidades, ordem e formato do histórico definidos no `design.md`. | QA-3 | FR-028, CA-020 |
| DEC-04 | Retomada após falha de leitura (sem latch): o bloqueio é removido com leitura válida, mas a carga não religa automaticamente — exige novo comando ON (interpretação conservadora do CONCERNS do `CODEBASE.md`). | Risco residual de religamento | FR-015, CA-016 |
| DEC-05 | SSID no formato `ESP8266_XXXXXX` (underscore): F1 prevalece sobre o exemplo com hífen de F2. | Conflito entre fontes | FR-018 |
| DEC-06 | Orçamentos de verificação adotados nesta spec: buzzer ± 20% por fase; resposta HTTP ≤ 500 ms; atualização da dashboard ≤ 2 s; desktop de referência 1366×768. São revisáveis no design, sem afrouxar NFR-003/NFR-004. | Critérios mensuráveis | NFR-003, NFR-004, FR-021, FR-029 |
| DEC-07 | Cadências e vínculo de sensor revisados por decisão do proprietário do produto (2026-09-12): amostragem nominal de 1200 ms ± 150 ms (desvio intencional da F1 §4.2, que previa 1 s; a conversão de 750 ms segue < período); dashboard com polling de 2000 ms e reflexão ≤ 2,5 s; conversões e leituras do DS18B20 vinculadas à ROM identificada na varredura (FR-001/FR-005), com re-varredura de 5 s revinculando. Revê parcialmente DEC-06 e é o critério vigente para CA-004/CA-023/NFR-003. | Decisão do proprietário (desvio da F1) | FR-001, FR-004, FR-005, FR-029, NFR-003, CA-004, CA-023 |

## 8. Premissas, riscos e pendências

### 8.1 Premissas

| ID | Premissa | Origem |
|---|---|---|
| P1 | Hardware físico de bancada disponível (placa em `/dev/ttyUSB0`, host com Wi-Fi, DS18B20, buzzer ativo, carga). | Contexto da tarefa / ROADMAP §3.1 |
| P2 | Um único DS18B20 no barramento; o endereço identificado na varredura é a referência do sensor. | F1 §4.1 |
| P3 | Pull-up do barramento OneWire **A CONFIRMAR** no hardware físico (típico 4,7 kΩ). Desde ADR-014 (2ª sessão) o firmware habilita o pull-up **interno** do GPIO4 como mitigação e faz retries de varredura/leitura; o resistor externo de 4,7 kΩ segue recomendado. | constitution §4 |
| P4 | Alimentação por USB 5 V, I/O em 3,3 V; GPIO16 restrito a `digitalRead/digitalWrite`. | constitution §1/§4 |
| P5 | O comportamento do DS18B20 segue a biblioteca `DallasTemperature` (erros de comunicação reportados pela biblioteca são tratados como leitura inválida). | F1 §4 / CODEBASE |
| P6 | Cenário de uso local de estudo de caso; AP aberto é decisão de produto registrada (risco aceito). | F1 §3 / CODEBASE (CONCERNS) |

### 8.2 Riscos

| ID | Risco | Mitigação | Relação |
|---|---|---|---|
| R1 | Corte/latch falham ou são contornáveis ⇒ risco térmico. | Política como lógica pura determinística; testes HOST de fronteira; rearme manual explícito (DEC-01); sessão de bancada. | ROADMAP R1 |
| R2 | Estouro de RAM/flash por buffers dinâmicos ou HTML pesado. | NFR-002; buffers estáticos; PROGMEM; medição de footprint; `/json` expõe RAM/flash para observação. | ROADMAP R2 |
| R3 | Jitter de amostragem sob carga HTTP. | Agendamento por `millis()` desacoplado do HTTP; conversão assíncrona; medição em CA-004/CA-021. | ROADMAP R3 |
| R4 | Itens só verificáveis fisicamente (buzzer, PWM real, OneWire, AP/DHCP, boot/watchdog). | Checklist de bancada (CA-013…CA-019, CA-021); proibido declarar BANCADA sem hardware. | ROADMAP R4 |
| R5 | Reset/watchdog por bloqueio acidental. | NFR-001; sessão contínua em CA-026; boot seguro em CA-018. | ROADMAP R5 |
| R6 | Deriva de requisitos na reimplementação. | Matriz de rastreabilidade (§9); revisão contra F1. | ROADMAP R6 |
| R7 | Latch volátil: reset limpa o latch e exige rearme após eventos; falha de sensor não é latcheada (retomada com leitura válida — DEC-04). | Documentado como comportamento esperado e risco residual (estado seguro por padrão). | CODEBASE (CONCERNS) |
| R8 | Estratégia de atingir 79,9/80,0 °C em bancada ainda não definida. | QA-4 (design/tasks); sem meio calibrado, verificação física fica limitada à injeção no env `bancada`. | ROADMAP QA-4 |

### 8.3 Perguntas não bloqueadoras (herdadas do ROADMAP)

| ID | Questão | Onde será decidida | Impacto se ausente |
|---|---|---|---|
| QA-4 | Como a bancada atinge a fronteira 79,9/80,0 °C com segurança (injeção, ensaio térmico real ou ambos)? | `design.md` + `tasks.md` (03/04) | Limita a evidência física de CA-008/CA-009 a injeção + HOST, com limitação registrada |
| QA-5 | Quais versões exatas de plataforma, framework e bibliotecas serão fixadas? | `design.md` / `firmware/platformio.ini` (03) | Bloqueia CA-027 até a fixação |

## 9. Matriz de rastreabilidade (atualizada em 2026-09-12)

> `Estado`: `PASS` = evidência registrada (HOST e/ou BANCADA); `PASS*` = PASS com nota/limitação explícita; `PENDENTE` = aguardando ação física do operador (justificativa na célula). Evidências em `docs/05-testing/controle-termico/` (checklist `2026-09-12-checklist-bancada.md`).
>
> **Revisão DEC-07 (2ª sessão de bancada, 2026-09-12):** cadências 1200 ms/2000 ms e vínculo de ROM em vigor (CA-004 [1050, 1350] ms; CA-023 ≤ 2,5 s); mitigação do barramento OneWire (pull-up interno + retries) registrada em **ADR-014**; evidências com sufixo `-dec07`/`-dec07b`.

| Requisito | Critério | Tarefa | Código | Teste | Evidência | Estado |
|---|---|---|---|---|---|---|
| FR-001 | CA-001, CA-002, CA-017 | M2–M3, T-019, T-022 | `src/ds18b20_sensor`, `src/main.cpp` | `firmware/test/test_control_policy` (estado de boot) + log serial + leitura por ROM | HOST + BANCADA (`...-boot-dec07c.log`, `...-soak-retry-dec07b.log`, `...-bateria-injecao-dec07.log`) | PASS — ROM identificada/vinculada no `setup` (`28FFE203B41605C2`, 0,28 s) e usada nas leituras; retries de varredura/leitura (ADR-014) eliminam as falhas do protótipo sem pull-up externo |
| FR-002 | CA-002, CA-007, CA-017 | M1–M2 | `lib/thermal_logic/control_policy`, `src/main.cpp` | `firmware/test/test_control_policy` | HOST + BANCADA em `docs/05-testing/` | PASS (HOST+BANCADA — ausência real: `NO_SENSOR`, ON recusado) |
| FR-003 | CA-003 | M2–M3, T-019 | `src/ds18b20_sensor`, `lib/thermal_logic/periodic_timer` | `firmware/test/test_periodic_timer` + desconexão/reconexão física | HOST + BANCADA (`...-sensor-instavel-dec07.log`, `...-boot-dec07.log`) | PASS — detecção e perda em runtime observadas (mesma ROM revinculada; carga bloqueada na perda); `rescan()` re-enumera com `begin()` (`getDeviceCount()` é cacheado — ADR-013) |
| FR-004 | CA-004, CA-021 | M1–M3, T-020 | `lib/thermal_logic/periodic_timer`, `src/config.h` | `firmware/test/test_periodic_timer` + medição de ciclo | HOST + BANCADA (`...-bateria-injecao-dec07.log` 1176–1224 ms; `...-rede-dec07.log` 1200 ms) | PASS (DEC-07 — janela [1050, 1350] ms) |
| FR-005 | CA-004, CA-005 | M2–M3, T-019 | `src/ds18b20_sensor` | inspeção (assíncrono por ROM) + estabilidade da amostragem | BANCADA (`...-rede-dec07.log`, `...-bateria-injecao-dec07.log`) | PASS (DEC-07 — `requestTemperaturesByAddress(rom)`/`getTempC(rom)`; sem espera; cadência mantida sob carga) |
| FR-006 | CA-015, CA-017 | M1 | `lib/thermal_logic/control_policy` | `firmware/test/test_control_policy` | HOST em `docs/05-testing/` | PASS (HOST + injeção `FAULT` na bancada) |
| FR-007 | CA-006, CA-007 | M2–M3, T-023 | `src/web_server`, `src/web/dashboard_html.h`, `lib/thermal_logic/control_policy` | `firmware/test/test_control_policy` + HTTP em bancada + clique real no navegador | HOST + BANCADA (`...-dashboard-botao-dec07d.log`) | PASS (HOST+BANCADA — `/on`/`/off` via HTTP reais; botões do dashboard funcionais após correção do handler) |
| FR-008 | CA-006 | M1–M3 | `lib/thermal_logic/control_policy`, `src/main.cpp` | `firmware/test/test_control_policy` + medição de PWM | HOST + BANCADA em `docs/05-testing/` | PASS (HOST+BANCADA — PWM 1023/0 observado no `/json`) |
| FR-009 | CA-007 | M1–M2 | `lib/thermal_logic/control_policy` | `firmware/test/test_control_policy` | HOST + BANCADA em `docs/05-testing/` | PASS (HOST+BANCADA — recusas com motivo) |
| FR-010 | CA-008, CA-009 | M1–M3 | `lib/thermal_logic/control_policy` | `firmware/test/test_control_policy` + injeção em bancada | HOST + BANCADA em `docs/05-testing/` | PASS (HOST+BANCADA — corte na mesma avaliação, bateria checks 4/10) |
| FR-011 | CA-010 | M1–M2 | `lib/thermal_logic/control_policy` | `firmware/test/test_control_policy` | HOST + BANCADA em `docs/05-testing/` | PASS (HOST+BANCADA — latch persiste após esfriar) |
| FR-012 | CA-011, CA-012 | M1–M3 | `lib/thermal_logic/control_policy`, `src/web_server`, `src/console` | `firmware/test/test_control_policy` + rearme em bancada | HOST + BANCADA em `docs/05-testing/` | PASS (HOST+BANCADA — `/rearm` aceito/recusado com motivo) |
| FR-013 | CA-013, CA-014 | M1–M3 | `lib/thermal_logic/buzzer_pattern` | `firmware/test/test_buzzer_pattern` + medição/audição | HOST + BANCADA em `docs/05-testing/` | PASS* (HOST nas fases 150/2000 ms + acionamento por injeção; audição do operador PENDENTE) |
| FR-014 | CA-015, CA-017 | M1–M3 | `lib/thermal_logic/control_policy`, `lib/thermal_logic/status_json` | `firmware/test/test_control_policy`, `firmware/test/test_status_json` | HOST + BANCADA (`...-sensor-instavel-dec07.log`) | PASS (HOST + injeção `FAULT` + perda física real: `NO_SENSOR`/`INVALID_READING`, PWM 0, estado explícito) |
| FR-015 | CA-016 | M1 | `lib/thermal_logic/control_policy` | `firmware/test/test_control_policy` | HOST em `docs/05-testing/` | PASS (HOST+BANCADA — sem religamento automático) |
| FR-016 | CA-001, CA-018 | M1–M3 | `lib/thermal_logic/control_policy`, `src/main.cpp` | `firmware/test/test_control_policy` + reset forçado | HOST + BANCADA em `docs/05-testing/` | PASS (HOST+BANCADA — reset limpa latch; carga desligada) |
| FR-017 | CA-018 | M2–M3 | `src/main.cpp` | inspeção + log serial em bancada | BANCADA em `docs/05-testing/` | PASS (motivo do reset no log de boot de produção) |
| FR-018 | CA-019 | M2–M3 | `src/main.cpp` (Wi-Fi/AP) | associação de cliente real em bancada | BANCADA em `docs/05-testing/` | PASS (AP aberto `ESP8266_101026` associado) |
| FR-019 | CA-019 | M2–M3 | `src/main.cpp` (Wi-Fi/AP) | DHCP/IP e `http://192.168.4.1` em bancada | BANCADA em `docs/05-testing/` | PASS (DHCP .100/24; dashboard e `/json`) |
| FR-020 | CA-021 | M2–M3 | `src/web_server` | polling sob alarme em bancada | BANCADA em `docs/05-testing/` | PASS (sessão ~60 s sob alarme sem reset) |
| FR-021 | CA-022 | M2–M3 | `src/web/dashboard_html.h` | checklist visual em bancada (1366×768) | BANCADA em `docs/05-testing/` | PASS (navegador real, sem rolagem) |
| FR-022 | CA-022 | M2–M3 | `src/web/dashboard_html.h` | checklist visual em bancada | BANCADA em `docs/05-testing/` | PASS (valor numérico + gauge renderizados) |
| FR-023 | CA-022 | M1–M3 | `src/web/dashboard_html.h`, `lib/thermal_logic/trend_buffer` | `firmware/test/test_trend_buffer` + checklist visual | HOST + BANCADA em `docs/05-testing/` | PASS (HOST + gráfico 20–90 °C com histórico real) |
| FR-024 | CA-007, CA-022 | M2–M3, T-023 | `src/web/dashboard_html.h`, `src/web_server` | checklist visual + recusa observada + clique real nos botões | BANCADA (`...-dashboard-botao-dec07d.log`) | PASS (cores de estado + feedback de recusa; botões Ligar/Desligar/Rearmar funcionais no navegador) |
| FR-025 | CA-020, CA-022 | M1–M3 | `src/web/dashboard_html.h`, `lib/thermal_logic/status_json`, `src/main.cpp` | `firmware/test/test_status_json` + checklist visual | HOST + BANCADA em `docs/05-testing/` | PASS (métricas idle/RAM/flash/uptime visíveis) |
| FR-026 | CA-022 | M2–M3 | `src/web/dashboard_html.h` | checklist visual (alarme/latch/rearme) | BANCADA em `docs/05-testing/` | PASS (alertas + latch + botão rearme) |
| FR-027 | CA-022 | M2 | `src/web/dashboard_html.h` | checklist visual (tooltips) | BANCADA em `docs/05-testing/` | PASS (23 tooltips medidos) |
| FR-028 | CA-020 | M1–M3 | `lib/thermal_logic/status_json` | `firmware/test/test_status_json` + captura do payload | HOST + BANCADA em `docs/05-testing/` | PASS (HOST + payload real completo) |
| FR-029 | CA-023 | M2–M3, T-020 | `src/web/dashboard_html.h`, `src/web_server` | medição de polling/reflexão em navegador real | BANCADA (`...-dashboard-dec07.log` + `.png`) | PASS (DEC-07 — polling 2000 ms; reflexão 1426/1996 ms ≤ 2,5 s; sem recarga) |
| FR-030 | CA-024 | M2–M3 | `src/console` | compilação dos dois envs + uso em bancada | HOST + BANCADA em `docs/05-testing/` | PASS (strings por env + bateria real) |
| NFR-001 | CA-026 | M1–M3 | todos os módulos (`src/`, `lib/thermal_logic/`) | inspeção estática + sessão contínua de bancada | HOST + BANCADA em `docs/05-testing/` | PASS (sem `delay()`/ISR/timers/persistência; sessões sem reset) |
| NFR-002 | CA-025 | M2 | `firmware/platformio.ini`, `src/web/dashboard_html.h`, módulos | medição de footprint + inspeção de alocação | HOST (build) em `docs/05-testing/` | PASS (31,2%/39,1%; metas ≤ 45%/≤ 50%) |
| NFR-003 | CA-004, CA-013, CA-021 | M1–M3, T-020 | `lib/thermal_logic/periodic_timer`, `lib/thermal_logic/buzzer_pattern`, `src/main.cpp` | `firmware/test/test_periodic_timer`, `firmware/test/test_buzzer_pattern` + medição | HOST + BANCADA (logs `-dec07`) | PASS* (DEC-07 — amostragem 1176–1224 ms sob carga; buzzer HOST; audição do operador PENDENTE) |
| NFR-004 | CA-021 | M3 | `src/web_server`, `src/main.cpp` | medição de latência HTTP sob alarme | BANCADA em `docs/05-testing/` | PASS (`/json` máx 26,6 ms; páginas 31–81 ms) |
| NFR-005 | CA-027 | M2 | `firmware/platformio.ini` (planejado) | build em checkout limpo | HOST (build) em `docs/05-testing/` | PASS* (versões fixadas; rebuild limpo validado) |
| NFR-006 | CA-028 | M1–M4 | documentação (`.specs/features/controle-termico/`, `docs/05-testing/`) | auditoria da matriz de rastreabilidade | HOST (documental) | PASS (matriz atualizada + checklist de bancada) |

Regras da matriz (`docs/agentic/TRACEABILITY.md`): não marcar `PASS` sem evidência; `PENDENTE` deve conter justificativa e risco residual; alteração de requisito propaga para critério, tarefa, teste e documentação.

## Referências

- `docs/descricao.txt` (F1) — fonte normativa do produto.
- `docs/estudo-de-caso.txt` (F2) — complemento do produto.
- `.specs/project/constitution.md` — princípios obrigatórios (segurança, determinismo, recursos, interfaces, qualidade).
- `.specs/project/ROADMAP.md` — escopo macro, objetivos, riscos, marcos e questões abertas (QA-4, QA-5).
- `.specs/codebase/CODEBASE.md` — stack, alvo, arquitetura alvo, convenções, testes e concerns.
- `AGENTS.md` — regras permanentes, restrições de hardware e comandos.
- `.github/skills/sdd-embarcado/SKILL.md` §2 — formato de especificação de comportamento.
- `docs/agentic/TRACEABILITY.md` — formato e regras da matriz de rastreabilidade.
