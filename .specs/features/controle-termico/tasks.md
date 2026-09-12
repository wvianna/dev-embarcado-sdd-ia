# tasks.md — Feature `controle-termico`

| Campo | Valor |
|---|---|
| Feature | `controle-termico` (reimplementação completa do firmware) |
| Data | 2026-09-12 |
| Agente | 04-task-planner |
| Base | `spec.md` (FR/NFR/CA) + `design.md` (módulos, ADRs, contratos) |
| Marcos | M1 lógica pura · M2 build/integração · M3 bancada · M4 fechamento (ROADMAP §6) |
| Estados | `[ ]` pendente · `[-]` em andamento · `[x]` concluída com evidência · `[!]` bloqueada |

## M1 — Lógica pura e testes HOST

### [x] T-001 Criar estrutura PlatformIO com versões fixadas

- **Requisitos:** NFR-005; habilita M1–M3.
- **Onde:** `firmware/platformio.ini` (envs `nodemcuv2`, `bancada`, `native`), esqueleto de `firmware/src/main.cpp`, `firmware/src/config.h`.
- **Depende de:** nenhum.
- **Reutiliza:** convenções de `AGENTS.md` §3/§4 (comandos `pio run -d firmware`, `pio test -d firmware -e native`).
- **Feito quando:** `espressif8266@4.2.1` + `OneWire@2.3.8` + `DallasTemperature@3.11.0` resolvidos; `nodemcuv2` compila esqueleto; env `bancada` herda + `-DBENCH_TEMP_INJECTION=1`; `native` disponível para testes.
- **Testes:** compilação (HOST/toolchain); `pio pkg list` registrando versões (evidência CA-027).
- **Gate:** `pio run -d firmware -e nodemcuv2` SUCCESS e `pio pkg list -d firmware -e nodemcuv2` com versões fixas.

### [x] T-002 Implementar `ControlPolicy` com testes de fronteira e latch

- **Requisitos:** FR-002, FR-006, FR-008, FR-009, FR-010, FR-011, FR-012, FR-014, FR-015, FR-016.
- **Onde:** `firmware/lib/thermal_logic/control_policy.{h,cpp}`; `firmware/test/test_control_policy.cpp`.
- **Depende de:** T-001.
- **Reutiliza:** ADR-003 (avaliação em ordem fixa; fail-safe limpa o comando); contrato §3.1 do design.
- **Feito quando:** 79,9 °C mantém ON; 80,0 °C ⇒ PWM 0 + latch + alarme na mesma avaliação; ON recusado em `no_sensor`/`invalid_reading`/`latched`; rearme só com válida < 80,0 °C (`not_latched`/`no_sensor`/`invalid_reading`/`temp_high` como recusa); pós-rearme/pós-retomada exige novo ON; boot bloqueia até leitura válida.
- **Testes:** Unity HOST — casos acima + prioridade de bloqueio `latched > no_sensor > invalid_reading` + limpeza fail-safe do comando.
- **Gate:** `pio test -d firmware -e native -f test_control_policy` verde.

### [x] T-003 Implementar `PeriodicTimer` com testes de wraparound

- **Requisitos:** FR-004; NFR-003.
- **Onde:** `firmware/lib/thermal_logic/periodic_timer.{h,cpp}`; `firmware/test/test_periodic_timer.cpp`.
- **Depende de:** T-001.
- **Reutiliza:** contrato §3.2 (comparação sem sinal, cadência sem drift, reancoragem).
- **Feito quando:** vence exatamente no período; não vence antes; mantém cadência após atraso; `millis()` perto de `UINT32_MAX` não quebra (wrap).
- **Testes:** Unity HOST com tempos sintéticos, incluindo `0xFFFFFF00`+.
- **Gate:** `pio test -d firmware -e native -f test_periodic_timer` verde.

### [x] T-004 Implementar `BuzzerPattern` com fases 150/2000 ms

- **Requisitos:** FR-013; NFR-003.
- **Onde:** `firmware/lib/thermal_logic/buzzer_pattern.{h,cpp}`; `firmware/test/test_buzzer_pattern.cpp`.
- **Depende de:** T-001.
- **Reutiliza:** contrato §3.3; constantes do design §6.
- **Feito quando:** condição falsa ⇒ saída falsa imediata e reinício de fase; condição verdadeira ⇒ ON em [0,150), OFF em [150,2150), repique em 2150; configuração do alarme no instante zero liga o buzzer.
- **Testes:** Unity HOST nos limites 149/150/2149/2150 e desligamento com condição falsa.
- **Gate:** `pio test -d firmware -e native -f test_buzzer_pattern` verde.

### [x] T-005 Implementar `TrendBuffer` com testes de circularidade

- **Requisitos:** FR-023.
- **Onde:** `firmware/lib/thermal_logic/trend_buffer.{h,cpp}`; `firmware/test/test_trend_buffer.cpp`.
- **Depende de:** T-001.
- **Reutiliza:** contrato §3.4 (owner externo, mais antigo→mais recente).
- **Feito quando:** push/ordenação corretos; ao encher, sobrescreve o mais antigo sem perder o mais recente; `clear()` zera; `at()` respeita os limites.
- **Testes:** Unity HOST com capacidade pequena (ex.: 4) e cheia.
- **Gate:** `pio test -d firmware -e native -f test_trend_buffer` verde.

### [x] T-006 Implementar `StatusJson` com contrato fixado

- **Requisitos:** FR-025, FR-028; NFR-002.
- **Onde:** `firmware/lib/thermal_logic/status_json.{h,cpp}`; `firmware/test/test_status_json.cpp`.
- **Depende de:** T-001.
- **Reutiliza:** tabela de campos §3.5 (ordem/ unidades/ tipos).
- **Feito quando:** todos os campos na ordem fixada; determinismo (mesmo snapshot ⇒ mesmo texto); truncamento de `hist` respeita `cap` (sem overflow, sem JSON inválido, `hist_count` coerente); temperatura representada em centésimos.
- **Testes:** Unity HOST — snapshot completo, buffer mínimo, histórico maior que o cap, estados de falha (`sensor=false`, `valid=false`).
- **Gate:** `pio test -d firmware -e native -f test_status_json` verde.

## M2 — Integração, build e footprint

### [x] T-007 Implementar driver `ds18b20_sensor`

- **Requisitos:** FR-001, FR-003, FR-005, FR-006, FR-014.
- **Onde:** `firmware/src/ds18b20_sensor.{h,cpp}`.
- **Depende de:** T-001.
- **Reutiliza:** ADR-002 (conversão assíncrona); pinagem `AGENTS.md` §5 (D2/GPIO4).
- **Feito quando:** `begin()` varre e registra ROM/ausência; conversão sem espera (`setWaitForConversion(false)`, resolução 12); `read()` classifica validade (−127, NaN, fora de [−55, +125]) e devolve centésimos; `rescan()` atualiza presença.
- **Testes:** HOST (compilação da lógica de validade não se aplica aqui — driver de plataforma); BANCADA via console/injeção e desconexão física (T-014/T-015).
- **Gate:** compilação `nodemcuv2`/`bancada` + leitura válida na bancada (T-013).

### [x] T-008 Integrar `main.cpp` (loop, atuação, Wi-Fi/AP, métricas)

- **Requisitos:** FR-004, FR-005, FR-010, FR-016, FR-017, FR-018, FR-019, FR-020; NFR-001.
- **Onde:** `firmware/src/main.cpp`.
- **Depende de:** T-002, T-003, T-004, T-005, T-006, T-007.
- **Reutiliza:** fluxo §4.2/§4.3; pinagem e restrições `AGENTS.md` §5.
- **Feito quando:** setup na ordem do design; loop sem bloqueio com atuação na mesma passada; AP aberto `ESP8266_XXXXXX` + IP fixo + DHCP; métricas `load/idle` por janela de 1 s; reset logado.
- **Testes:** compilação + bancada (T-013…T-015).
- **Gate:** `pio run -d firmware -e nodemcuv2` SUCCESS; AP associável na bancada (T-014).

### [x] T-009 Implementar `web_server` e `dashboard_html`

- **Requisitos:** FR-007, FR-008, FR-009, FR-012, FR-020, FR-021, FR-022, FR-023, FR-024, FR-025, FR-026, FR-027, FR-029; NFR-004.
- **Onde:** `firmware/src/web_server.{h,cpp}`, `firmware/src/web/dashboard_html.h`.
- **Depende de:** T-006, T-008.
- **Reutiliza:** rotas/contrato §4.5 e ADR-010 (sem CDN, tooltips, polling 1 s, layout 1366×768).
- **Feito quando:** `/`, `/json`, `/on`, `/off`, `/rearm`, 404 conforme contrato; dashboard com gauge, gráfico 20–90 °C (grade cinza, limite 80 °C), destaques, alertas, latch+rearme, métricas, tooltips; AJAX ≤ 2 s.
- **Testes:** HOST (compilação); BANCADA — capturas da página e do payload, checklist visual (CA-022/CA-023).
- **Gate:** rotas respondendo na bancada (T-014) + checklist visual (T-015).

### [x] T-010 Implementar console de bancada

- **Requisitos:** FR-030.
- **Onde:** `firmware/src/console.{h,cpp}`; integração condicional em `main.cpp`.
- **Depende de:** T-006, T-008.
- **Reutiliza:** ADR-007 (somente `bancada`) e protocolo §4.6.
- **Feito quando:** comandos `TEMP/FAULT/REAL/ON/OFF/REARM/STATE/RESCAN/REBOOT/HELP` respondem `OK/ERR/JSON` em uma linha; env `nodemcuv2` não contém o console.
- **Testes:** compilação nos 2 envs; uso na bancada (T-013).
- **Gate:** `nm`/inspeção ausente no `nodemcuv2` + bateria da bancada executando os comandos.

### [x] T-011 Criar script de bancada `bench_injection_test.py`

- **Requisitos:** FR-030, CA-024; suporta CA-008…CA-016.
- **Onde:** `firmware/tools/bench_injection_test.py`.
- **Depende de:** T-010.
- **Reutiliza:** bateria §4.7 do design.
- **Feito quando:** executa a bateria completa, valida JSON de `STATE`, imprime veredito por verificação e sai com código ≠ 0 em falha.
- **Testes:** execução real na bancada (T-013).
- **Gate:** bateria 10/10 verde com log salvo em `docs/05-testing/`.

### [x] T-012 Medir footprint e inspecionar memória no build alvo

- **Requisitos:** NFR-002, NFR-005.
- **Onde:** build `nodemcuv2`; registro em `docs/05-testing/`.
- **Depende de:** T-008, T-009.
- **Reutiliza:** metas da constituição §3 (≤ 45% sketch, ≤ 50% RAM).
- **Feito quando:** sketch e RAM medidos e dentro das metas; inspeção confirma PROGMEM, buffers estáticos e ausência de `String` em acúmulo no loop.
- **Testes:** build + inspeção (evidência HOST).
- **Gate:** `pio run -d firmware -e nodemcuv2` SUCCESS com números registrados.

## M3 — Bancada (hardware real)

### [x] T-013 Gravar env `bancada` e executar bateria de injeção

- **Requisitos:** CA-008, CA-009, CA-010, CA-011, CA-012, CA-013, CA-014, CA-015, CA-016, CA-024; FR-010…FR-015, FR-030.
- **Onde:** placa em `/dev/ttyUSB0`; `pio run -d firmware -e bancada -t upload --upload-port /dev/ttyUSB0`; `firmware/tools/bench_injection_test.py`.
- **Depende de:** T-011, T-012.
- **Reutiliza:** bateria §4.7; console §4.6.
- **Feito quando:** proteção verificada no firmware real: fronteira 79,9/80,0 °C, latch, recusas de ON/rearme, persistência após esfriar, retomada sem religamento.
- **Testes:** BANCADA por injeção (carga pode estar desconectada para segurança elétrica).
- **Gate:** bateria 10/10 verde + log serial preservado.

### [x] T-014 Validar AP/DHCP/HTTP e jitter com carga real

- **Requisitos:** CA-004, CA-019, CA-020, CA-021, CA-023; FR-004, FR-018, FR-019, FR-020, FR-029; NFR-003, NFR-004.
- **Onde:** host Wi-Fi conectado ao AP do ESP; `curl`/script; logs do firmware.
- **Depende de:** T-013.
- **Reutiliza:** contrato `/json`; métricas `samp_*` para evidência de jitter.
- **Feito quando:** cliente associa e recebe DHCP 192.168.4.x; `/` e `/json` respondem; polling ≥ 1 Hz por ≥ 60 s sob alarme injetado com todas as respostas ≤ 500 ms; `samp_min/max` dentro de [850, 1150] ms; sem reset.
- **Testes:** BANCADA (medições registradas).
- **Gate:** log de latências + JSONs capturados + estatísticas de amostragem.

### [!] T-015 Validar física assistida e visual (operador) — PARCIAL

- **Requisitos:** CA-002, CA-003, CA-013, CA-014, CA-017, CA-018, CA-022.
- **Onde:** placa real; interação do operador (desconectar D2, ouvir buzzer, reset, inspeção visual 1366×768).
- **Depende de:** T-013, T-014.
- **Reutiliza:** roteiro §8.3 do design.
- **Feito quando:** sensor ausente detectado e re-varredura reconecta; buzzer audível no ciclo; reset forçado volta seguro (latch limpo, bloqueado até leitura válida); dashboard conforme checklist visual.
- **Testes:** BANCADA física assistida (o que não for executado fica `PENDENTE` com justificativa).
- **Gate:** checklist preenchido e assinado com o que foi observado.
- **Status 2026-09-12:** visual (CA-022/CA-023), ausência real de sensor (CA-002/CA-017) e reset (CA-018) executados; **pendentes de operador**: detecção/reconexão do DS18B20 (CA-001/CA-003), audição do buzzer (CA-013) e variante física de CA-015 — roteiro no `HANDOFF.md`.

### [x] T-016 Regravar build final `nodemcuv2` (produção)

- **Requisitos:** CA-024 (compilação sem console).
- **Onde:** `pio run -d firmware -t upload --upload-port /dev/ttyUSB0` (env default).
- **Depende de:** T-015.
- **Reutiliza:** gate de upload.
- **Feito quando:** placa opera com build de produção; console de injeção inexistente; AP/dashboard funcionais; estado final registrado.
- **Testes:** BANCADA (smoke de boot/AP).
- **Gate:** log de boot do build final + `/json` respondendo.

## M4 — Fechamento

### [x] T-017 Consolidar evidências, rastreabilidade e continuidade

- **Requisitos:** NFR-006; CA-028 (suporte).
- **Onde:** `docs/05-testing/`, matriz de rastreabilidade em `spec.md` §9, `STATUS.md`, `HANDOFF.md`.
- **Depende de:** T-012…T-016.
- **Reutiliza:** `docs/agentic/TRACEABILITY.md` e `DEFINITION-OF-DONE.md`.
- **Feito quando:** cada CA executado tem evidência referenciada; estados atualizados (`PASS`/`PENDENTE` com justificativa); `STATUS.md` com decisões/lições; `HANDOFF.md` com pendências físicas e riscos residuais.
- **Testes:** auditoria documental (HOST).
- **Gate:** matriz atualizada + documentos revisados no diff.

## M5 — Revisão DEC-07: vínculo de ROM e cadências (2ª sessão de bancada, 2026-09-12)

> Substitui as referências de 1000 ms/1 s e de leitura por índice das tarefas M2–M3. Decisões: `spec.md` DEC-07 e `design.md` ADR-013.

### [x] T-018 Atualizar artefatos SDD e documentos normativos (DEC-07/ADR-013)

- **Requisitos:** FR-001, FR-004, FR-005, FR-029, NFR-003.
- **Onde:** `spec.md`, `design.md`, `constitution.md`, `AGENTS.md`, `ROADMAP.md`, `CODEBASE.md`, `README.md`.
- **Feito quando:** amostragem 1200 ms ± 150 ms, polling 2000 ms (reflexão ≤ 2,5 s) e vínculo de ROM registrados; CA-004 passa a [1050, 1350] ms.
- **Gate:** documentos revisados no diff; fontes normativas sem referência vigente a 1000 ms.

### [x] T-019 Vincular leitura/conversão à ROM e corrigir a re-varredura

- **Requisitos:** FR-001, FR-003, FR-005, FR-014.
- **Onde:** `firmware/src/ds18b20_sensor.{h,cpp}`.
- **Feito quando:** varredura do boot vincula a ROM (`getAddress`); conversão por `requestTemperaturesByAddress(rom)` e leitura por `getTempC(rom)`; `rescan()` re-enumera com `begin()` (o `getDeviceCount()` é cacheado) e revincula/limpa o vínculo; log de ROM registrado.
- **Testes:** bancada — ROM `28FFE203B41605C2` vinculada; leituras válidas 36–37 °C; perda/retomada em runtime com carga bloqueada.
- **Gate:** builds SUCCESS; evidências `...-bateria-injecao-dec07.log`, `...-rede-dec07.log`, `...-sensor-instavel-dec07.log`.

### [x] T-020 Revisar cadências (amostragem 1200 ms; dashboard 2000 ms)

- **Requisitos:** FR-004, FR-029, NFR-003.
- **Onde:** `firmware/src/config.h`, `firmware/src/web/dashboard_html.h`, `firmware/tools/bench_injection_test.py`, `firmware/src/main.cpp` (comentários).
- **Feito quando:** `kSamplePeriodMs = 1200`; `setInterval(refresh, 2000)`; janela da bateria [1050, 1350] ms.
- **Testes:** bateria check 13 (`min=1176 max=1224`); polling de 60 s (`samp=1200`); Resource Timing 2000 ms; reflexão 1426/1996 ms.
- **Gate:** evidências `...-bateria-injecao-dec07.log`, `...-rede-dec07.log`, `...-dashboard-dec07.log`.

### [x] T-021 Revalidar bancada, regravar produção e atualizar rastreabilidade

- **Requisitos:** CA-004, CA-021, CA-023, CA-024, CA-025, CA-028.
- **Onde:** placa `/dev/ttyUSB0`; `docs/05-testing/controle-termico/`; matriz `spec.md` §9; `STATUS.md`/`HANDOFF.md`.
- **Feito quando:** HOST 37/37; bateria 14/14; rede e dashboard medidos; produção regravada com estado final seguro documentado; matriz e documentos atualizados.
- **Gate:** logs `-dec07` + `2026-09-12-dashboard-dec07.png`; pendências físicas explícitas (pull-up P3; audição do buzzer).

### [x] T-022 Mitigação do barramento OneWire: pull-up interno + retries (ADR-014)

- **Requisitos:** FR-001, FR-003, FR-006, FR-009, FR-014 (sintomas de bancada: leituras intermitentes e `ON` recusado/limpo).
- **Onde:** `firmware/src/ds18b20_sensor.cpp`; design `ADR-014`; constitution §4/P3; `AGENTS.md` §5; `CODEBASE.md`.
- **Depende de:** T-019 (vínculo de ROM) e T-020 (cadências).
- **Feito quando:** pull-up interno do GPIO4 habilitado antes da varredura; 3 tentativas de varredura no boot e 2 por re-varredura; 1 retry imediato na leitura; `parasita=` no log da ROM.
- **Testes:** bancada — antes: 13 leituras inválidas/90 s e scan do boot falhando; depois: 0 inválidas/0 ausências em 90 s, `ON` retido com aquecimento 41,5 → 66,1 °C, boot vinculando a ROM no `setup` (0,28 s). Diagnóstico de fase do boot documentado (hipótese de conversão em andamento refutada).
- **Gate:** evidências `2026-09-12-soak-baseline-dec07b.log`, `...-soak-pullup-interno-dec07b.log`, `...-soak-final-dec07b.log`, `...-soak-retry-dec07b.log`, `...-diagnostico-fase-boot-dec07b.log`, `...-diagnostico-rescan-dec07b.log`, `...-diagnostico-on-dec07b.log`; produção regravada (`...-boot-producao-dec07c.log`).

## M6 — Correção do dashboard web (3ª sessão, 2026-09-12)

### [x] T-023 Corrigir o handler dos botões do dashboard (`cmd is not defined`)

- **Requisitos:** FR-007, FR-024, FR-029 (observáveis do operador).
- **Onde:** `firmware/src/web/dashboard_html.h`.
- **Depende de:** T-009.
- **Feito quando:** os botões Ligar/Desligar/Rearmar deixam de usar `onclick` inline (o `cmd` do IIFE não é global) e passam a usar `addEventListener` dentro do IIFE.
- **Testes:** navegador real — pré-correção: `Uncaught ReferenceError: cmd is not defined` e nenhum efeito; pós-correção: Ligar → PWM 1023 (aquecimento 41,2 → 47,9 °C), Desligar → PWM 0, Rearmar → recusa `not_latched` com feedback.
- **Gate:** `2026-09-12-dashboard-botao-dec07d.log` + screenshot `2026-09-12-dashboard-botao-carga-ligada.png`; produção regravada.

## Entregáveis e aceite

**Arquivos esperados**

- Código: `firmware/platformio.ini`, `firmware/src/{config.h,main.cpp,ds18b20_sensor.*,web_server.*,web/dashboard_html.h,console.*}`, `firmware/lib/thermal_logic/{control_policy,periodic_timer,buzzer_pattern,trend_buffer,status_json}.{h,cpp}`, `firmware/tools/bench_injection_test.py`.
- Testes: `firmware/test/test_{control_policy,periodic_timer,buzzer_pattern,trend_buffer,status_json}.cpp` (env `native`).
- Documentação: este `tasks.md`, `design.md`, `spec.md`, evidências em `docs/05-testing/controle-termico/`, `STATUS.md`, `HANDOFF.md` (na raiz).

**Comandos**

| Ação | Comando |
|---|---|
| Testes HOST (gate mínimo) | `pio test -d firmware -e native` |
| Build alvo | `pio run -d firmware` |
| Gravar bancada | `pio run -d firmware -e bancada -t upload --upload-port /dev/ttyUSB0` |
| Bateria de injeção | `python3 firmware/tools/bench_injection_test.py --port /dev/ttyUSB0` |
| Gravar produção | `pio run -d firmware -t upload --upload-port /dev/ttyUSB0` |

**Critérios de aceite rastreados**

- Fronteira térmica: CA-008/CA-009 (79,90 °C mantém; 80,00 °C corta + latch na mesma avaliação).
- Latch/rearme: CA-010, CA-011, CA-012; fail-safe sem religamento: CA-016.
- Timing: CA-004 (1200 ms ± 150 ms — DEC-07), CA-013 (150/2000 ms ± 20%), CA-021 (HTTP ≤ 500 ms + amostragem estável).
- Memória: CA-025 (≤ 45% sketch, ≤ 50% RAM) e NFR-002 (sem alocação no loop).
- Rede/UI: CA-019, CA-020, CA-022, CA-023; diagnóstico: CA-024; reprodutibilidade: CA-027.

**Pendências e riscos residuais**

- Validação física assistida (T-015) depende do operador; itens não executados viram `PENDENTE` com justificativa e risco residual em `HANDOFF.md`.
- Ensaio térmico real na fronteira de 80 °C **não** é evidência obrigatória (decisão QA-4/§8.4 do design); se realizado, substitui a injeção como evidência primária de CA-008/CA-009.
- Latch volátil (R7) e pull-up OneWire (P3) seguem como riscos documentados.
- **Responsável pela validação física:** operador humano (William), com o firmware instrumentado (console `bancada`) e este roteiro.
