# design.md — Feature `controle-termico`

| Campo | Valor |
|---|---|
| Feature | `controle-termico` (reimplementação completa do firmware) |
| Data | 2026-09-12 |
| Agente | 03-architect |
| Fontes | `spec.md` desta feature (comportamento observável), `docs/descricao.txt` (F1, normativa), `.specs/project/constitution.md`, `.specs/project/ROADMAP.md`, `.specs/codebase/CODEBASE.md`, `AGENTS.md` |
| Próximo artefato | `tasks.md` (04 — decomposição e gates) |
| Resolve | QA-1…QA-5 (ROADMAP §8): decisões em §9 (ADR-001…ADR-011) e §8 (QA-4) |

## 1. Princípios de design

1. **Fail-safe primeiro.** Toda condição de dúvida (sensor ausente, leitura inválida) bloqueia a carga; nenhum caminho religa sozinho. O corte por ≥ 80,0 °C é latcheado e só sai por rearme manual com leitura válida < 80,0 °C.
2. **Toda a segurança é lógica pura.** FSM, agendamento, padrão do buzzer, tendência e JSON vivem em `firmware/lib/thermal_logic/` **sem** `Arduino.h` — o que garante verificação HOST determinística do requisito mais crítico.
3. **Estritamente não bloqueante.** Loop cooperativo com `millis()`; nenhum `delay()`, ISR ou timer de hardware; conversão DS18B20 assíncrona com leitura ao fim do ciclo e re-disparo imediato.
4. **Memória estática.** Nada é alocado no loop; buffers de tamanho fixo com owner único (loop principal); HTML/CSS/JS em PROGMEM; sem `String`/`std::string`/`std::vector` no alvo.
5. **Contrato observável estável.** `/json`, rotas de comando e protocolo do console são contratos versionados neste documento; a dashboard consome apenas o contrato, nunca o estado interno.

## 2. Arquitetura de módulos

```text
firmware/
├── platformio.ini                  # envs: nodemcuv2 (default), bancada, native
├── lib/thermal_logic/              # LÓGICA PURA — sem Arduino.h (testável em HOST)
│   ├── control_policy.{h,cpp}      # FSM de segurança: corte, latch, rearme, bloqueios
│   ├── periodic_timer.{h,cpp}      # agendamento não bloqueante por millis() (wrap-safe)
│   ├── buzzer_pattern.{h,cpp}      # padrão 150 ms ON / 2000 ms OFF
│   ├── trend_buffer.{h,cpp}        # histórico circular de temperatura (int16 centésimos)
│   └── status_json.{h,cpp}         # serialização do contrato /json em buffer estático
├── src/                            # PLATAFORMA — Arduino/ESP8266 SDK
│   ├── config.h                    # pinos, tempos, limiares, versão, capacidades
│   ├── main.cpp                    # composição, loop, atuação, métricas, Wi-Fi/AP
│   ├── ds18b20_sensor.{h,cpp}      # OneWire + DallasTemperature (async, scan, validade)
│   ├── web_server.{h,cpp}          # rotas HTTP e semântica de comandos
│   ├── web/dashboard_html.h        # dashboard em PROGMEM (HTML/CSS/JS, sem CDN)
│   └── console.{h,cpp}             # console de bancada (somente env bancada)
├── test/                           # Unity (env native) — um arquivo por módulo puro
│   ├── test_control_policy.cpp
│   ├── test_periodic_timer.cpp
│   ├── test_buzzer_pattern.cpp
│   ├── test_trend_buffer.cpp
│   └── test_status_json.cpp
└── tools/
    └── bench_injection_test.py     # bateria de proteção em bancada (pyserial)
```

Dependências (direção única, sem ciclos):

```text
main.cpp ──usa──▶ ds18b20_sensor, web_server, console(bancada), lib/thermal_logic
web_server ──usa──▶ lib/thermal_logic (comandos e leitura de estado), dashboard_html.h
console ──usa──▶ lib/thermal_logic (comandos, JSON)          [somente bancada]
lib/thermal_logic ──não depende de nada além de <stdint.h>/<stddef.h>/<cstdio>
```

### 2.1 Papéis e costuras de teste

| Módulo | Responsabilidade | Entradas | Saídas | Teste HOST |
|---|---|---|---|---|
| `ControlPolicy` | Decidir bloqueio, latch, alarme e PWM a partir de fatos (presença/validade/temperatura) e comandos | `update(PolicyInputs, now)`, `requestHeater(on)`, `requestRearm()` | `pwm`, `latch`, `alarm`, `state`, `block/reason` | `test_control_policy.cpp` |
| `PeriodicTimer` | Vencer períodos sem drift e sem overflow | `now_ms` | `consume() == true` | `test_periodic_timer.cpp` |
| `BuzzerPattern` | Fase física do buzzer sob condição de alarme | `now_ms`, `condition` | `buzzer_on` | `test_buzzer_pattern.cpp` |
| `TrendBuffer` | Histórico circular para o gráfico | `push(centi)` | `size()`, `at(i)`, `newest()` | `test_trend_buffer.cpp` |
| `status_json` | Serializar o contrato (campos, unidades, ordem) em buffer estático com limite | `StatusSnapshot` | `size_t` (bytes escritos, sem overflow) | `test_status_json.cpp` |
| `ds18b20_sensor` | Varredura, conversão assíncrona, validade | OneWire/DallasTemperature | `present`, `Reading{valid, centi}` | Bancada (física) |
| `web_server` | Rotas e semântica de comando/resposta | HTTP | JSON de resposta, página | Bancada |
| `console` | Injeção/leitura/comandos de bancada | Linhas seriais | `OK/ERR/JSON` | Bancada (+ compilação dos 2 envs) |

## 3. Lógica pura (`lib/thermal_logic`)

### 3.1 `ControlPolicy`

Estados observáveis (`state()`): `NO_SENSOR` → `INVALID_READING` → `NORMAL` ⇄ `LATCHED`. O latch é ortogonal à condição do sensor: uma falha durante o latch mantém `LATCHED` (o bloqueio não é enfraquecido); `sensor`/`valid` permanecem visíveis como campos independentes no JSON e na UI.

Avaliação a cada amostra (`update`), em ordem fixa — a ordem é um requisito de segurança:

1. `alarm = sensor_present && reading_valid && temp_centi >= 8000` (fronteira inclusiva: ≥ 80,0 °C);
2. se `alarm` ⇒ `latch = true` (corte na **mesma** avaliação);
3. bloqueio = `LATCHED` > `NO_SENSOR` > `INVALID_READING` > nenhum (prioridade nessa ordem);
4. se bloqueado ⇒ `heater_command = false` (**fail-safe**: nada religa automaticamente; novo ON é obrigatório após qualquer desbloqueio — DEC-04).

Comandos e recusas (motivos observáveis: `none`, `no_sensor`, `invalid_reading`, `latched`, `not_latched`, `temp_high`):

| Comando | Condição de aceite | Recusa |
|---|---|---|
| `requestHeater(true)` (ON) | sem bloqueio | `no_sensor` \| `invalid_reading` \| `latched`; PWM permanece 0 |
| `requestHeater(false)` (OFF) | sempre | — |
| `requestRearm()` | `latch` ativo E `sensor_present` E `reading_valid` E `temp_centi < 8000` | `not_latched` \| `no_sensor` \| `invalid_reading` \| `temp_high`; latch mantido |

Saídas: `pwm() = (heater_command && !blocked) ? 1023 : 0`; `alarm()`; `latch()`; `state()`; `blockReason()`. `begin()` parte de: carga off, latch limpo, sem leitura válida (bloqueio até a primeira leitura).

### 3.2 `PeriodicTimer`

Baseado em **deadline** (instante do próximo vencimento, `begin = now + período`): `due(now)` compara `(int32_t)(now − deadline) >= 0` — diferença sinalizada, correta no wrap de `millis()` e imune a consumos repetidos no mesmo milissegundo (o loop pode iterar várias vezes por ms). `consume(now)` avança `deadline += período` (cadência sem drift) e, se ainda estiver vencido (atraso ≥ 1 período por loop preso), reancora em `now + período` — sem rajada de catch-up e sem duplo disparo. Instância declarada em `main` com período `kSamplePeriodMs` (1200 ms — DEC-07/ADR-013).

### 3.3 `BuzzerPattern`

Uso: `update(now_ms, condition)` → estado físico do pino. `condition=false` ⇒ saída `false` imediatamente (e reinício de fase na próxima ativação, garantindo início em ON). `condition=true` ⇒ ciclo `150 ms ON / 2000 ms OFF` (período 2150 ms), calculado por aritmética de wraparound. Não há estado no pino além deste módulo (sem ISR).

### 3.4 `TrendBuffer`

Owner da memória é o chamador: `TrendBuffer(int16_t* storage, size_t capacity)`; `main` fornece vetor estático `g_trend_storage[kTrendCapacity]` (`kTrendCapacity = 120` ⇒ 144 s a 1,2 s — DEC-07). `push` sobrescreve o mais antigo quando cheio; `at(0)` é o mais antigo, `at(size-1)` o mais recente. Somente leituras válidas entram no histórico.

### 3.5 `status_json` — contrato de `/json` (DEC-03)

Campos, ordem, tipos e unidades (ordem fixa; valores inteiros, sem `float`; temperatura em centésimos de °C `int16_t`):

| # | Campo | Tipo | Unidade | Fonte |
|---|---|---|---|---|
| 1 | `fw` | string | — | `config.h` (FW_VERSION) |
| 2 | `uptime_ms` | uint32 | ms | `millis()` |
| 3 | `state` | string | — | policy: `NO_SENSOR`/`INVALID_READING`/`NORMAL`/`LATCHED` |
| 4 | `sensor` | bool | — | presença (varredura OneWire) |
| 5 | `valid` | bool | — | validade da última leitura |
| 6 | `temp` | int16 | centésimos °C | última leitura válida; congelado quando inválida (nunca inventa valor) |
| 7 | `pwm` | uint16 | 0–1023 | aplicado à carga |
| 8 | `latch` | bool | — | latch de segurança |
| 9 | `alarm` | bool | — | `valid && temp ≥ 8000` |
| 10 | `block` | string | — | motivo do bloqueio (`none` quando liberado) |
| 11 | `load_pct` | uint8 | % | ocupação da janela de 1 s pelas tarefas do loop (§5.1) |
| 12 | `idle_pct` | uint8 | % | `100 − load_pct` |
| 13 | `ram_free` | uint32 | B | `ESP.getFreeHeap()` |
| 14 | `heap_frag` | uint8 | % | `ESP.getHeapFragmentation()` |
| 15 | `flash_used` | uint32 | B | `ESP.getSketchSize()` |
| 16 | `flash_pct` | uint8 | % | `flash_used × 100 / 1044464` |
| 17 | `samp_ms` | uint32 | ms | último intervalo de amostragem (evidência CA-004) |
| 18 | `samp_min_ms` | uint32 | ms | mínimo desde o boot |
| 19 | `samp_max_ms` | uint32 | ms | máximo desde o boot |
| 20 | `samp_win_min_ms` | uint32 | ms | mínimo dos últimos ≤ 60 intervalos (jitter em regime, pós-transiente de boot) |
| 21 | `samp_win_max_ms` | uint32 | ms | máximo dos últimos ≤ 60 intervalos |
| 22 | `ssid` | string | — | SSID do AP |
| 23 | `ip` | string | — | IP do AP |
| 24 | `clients` | uint8 | — | clientes associados (`softAPgetStationNum`) |
| 25 | `reset` | string | — | `ESP.getResetReason()` |
| 26 | `hist_period_ms` | uint32 | ms | período nominal do histórico |
| 27 | `hist_count` | size_t→uint16 | — | amostras válidas no histórico |
| 28 | `hist` | array int16 | centésimos °C | mais antigo → mais recente |

O builder nunca ultrapassa `cap`: ao esgotar espaço para o array `hist`, trunca o histórico e ajusta `hist_count` ao que foi emitido (sem inválidos/overrun), sempre com encerramento e `}` válidos. `main` usa buffer estático de 1600 B (§6).

## 4. Integração de plataforma (`src`)

### 4.1 `config.h` (fonte única de pinos, tempos e limiares)

`PIN_ONEWIRE=4` (D2), `PIN_BUZZER=16` (D0), `PIN_HEATER=5` (D1); `HEATER_PWM_MAX=1023`; `SAFETY_TEMP_CENTI=8000`; `SAMPLE_PERIOD_MS=1200` (DEC-07); `SENSOR_RESCAN_MS=5000`; `BUZZER_ON_MS=150`; `BUZZER_OFF_MS=2000`; `SERIAL_BAUD=115200`; `AP_IP=192.168.4.1/24`; `HTTP_PORT=80`; `TREND_CAPACITY=120`; `JSON_BUF_SIZE=1600`; `FW_VERSION="1.0.0"`; `HEALTH_WINDOW_MS=1000`. As constantes numéricas da lógica pura (limiar, período do buzzer, capacidade) são verificadas por teste HOST contra `config.h` (que inclui os headers da lib para `static_assert` de igualdade).

### 4.2 `main.cpp` — setup

1. `Serial.begin(115200)`; banner com versão e `ESP.getResetReason()` (FR-017).
2. Pinos em estado seguro: buzzer LOW; `pinMode`; `analogWriteRange(1023)`; `analogWrite(PIN_HEATER, 0)`.
3. `policy.begin()`; `buzzer.begin()`; timers ancorados em `millis()`.
4. Varredura OneWire (`sensor.begin()`): registra endereço ROM ou ausência (FR-001); se presente, dispara a 1ª conversão assíncrona.
5. Wi-Fi: `WiFi.persistent(false)` (sem gravação em flash); `mode(WIFI_AP)`; `setSleepMode(WIFI_NONE_SLEEP)` (elimina o atraso de power-save de ~3 s na primeira requisição após ociosidade — NFR-004); `softAPConfig(192.168.4.1, idem, /24)`; SSID `ESP8266_<MAC[3..5] maiúsculo>`; AP aberto (FR-018/FR-019).
6. `web_server.begin()` (rotas §4.4); `console.begin()` (somente bancada).

### 4.3 `main.cpp` — loop (ordem fixa, uma passada por iteração)

1. `now = millis()`; início da contabilidade de carga (§5.1).
2. **Amostragem** (timer de 1,2 s vencido — DEC-07): `r = sensor.read()` (resultado da conversão anterior — **nenhuma espera**); medir intervalo `now − last_sample` e atualizar `samp_ms/min/max`; `policy.update({present, r.valid, r.centi}, now)`; se válida, `trend.push`; se presente, `sensor.startConversion()` **imediatamente** (FR-005). No env `bancada`, a injeção substitui `r` e a presença (§4.6).
3. **Re-varredura** (timer de 5 s vencido, FR-003): `sensor.rescan()` — atualiza presença; ausente ⇒ nada é lido nem convertido.
4. **HTTP**: `web_server.poll()` → `handleClient()` (sem `delay`); respostas ≤ 500 ms por construção (payload em buffer estático, HTML em PROGMEM).
5. **Console** (somente bancada): leitura não bloqueante de linha (§4.6).
6. **Atuação**: `analogWrite(PIN_HEATER, policy.pwm())`; `digitalWrite(PIN_BUZZER, buzzer.update(now, policy.alarm()))` — a atuação segue a mesma avaliação da amostra (FR-010).
7. Fim da contabilidade de carga; janela de 1 s fechada ⇒ `load_pct/idle_pct` (§5.1).

### 4.4 `ds18b20_sensor` — varredura, vínculo de ROM e conversão assíncrona (FR-001/FR-005, DEC-07/ADR-013)

```text
setup   varredura do barramento ─ getAddress(0) ─▶ ROM vinculada (log serial)
t=0     tick: read() do ciclo anterior ─ requestTemperaturesByAddress(ROM) ─▶ conversão (750 ms, 12 bits)
t=750   conversão pronta (paralela ao loop; nada espera)
t=1200  tick: read() por getTempC(ROM) entrega 750 ms prontos ─ re-disparo imediato ─▶ ...
```

`setWaitForConversion(false)`; `setResolution(12)`. Conversões e leituras usam a **ROM vinculada** na varredura (`requestTemperaturesByAddress`/`getTempC(addr)`), não o índice do barramento. Validade: `DEVICE_DISCONNECTED_C` (−127), `NaN` ou fora de [−55, +125] °C ⇒ inválida (FR-006); centésimos por `lroundf(t × 100)`. **Re-varredura (5 s):** `getDeviceCount()` do DallasTemperature é **cacheado** (não re-varre); `rescan()` chama `begin()` para reenumerar o barramento de fato e revincula a ROM (sensor substituído ⇒ novo endereço; ausente ⇒ vínculo limpo). Falha física de leitura **não** é erro de firmware: vira `valid=false` e a política bloqueia (FR-014).

### 4.5 `web_server` — rotas e semântica (contrato HTTP)

| Rota | Método | Resposta | Semântica |
|---|---|---|---|
| `/` | GET | `200 text/html` (PROGMEM) | Dashboard (§4.5.1) |
| `/json` | GET | `200 application/json` | Contrato §3.5 (buffer estático) |
| `/on` | GET | `200 application/json` | `policy.requestHeater(true)` → `{"ok":..,"pwm":..,"latch":..,"reason":"..", "state":".."}` |
| `/off` | GET | `200 application/json` | `policy.requestHeater(false)` (sempre aceito) |
| `/rearm` | GET | `200 application/json` | `policy.requestRearm()` com motivo da recusa |
| outra | * | `404 text/plain` | — |

Comandos são avaliados **sincronamente** no handler (mesmo contexto do loop — §5), com atuação aplicada na mesma iteração seguinte; `ok=false` + `reason` dão o feedback explícito exigido por FR-009/FR-012/FR-024.

#### 4.5.1 `web/dashboard_html.h` (FR-021…FR-027, FR-029)

- pt-BR, tema claro, fontes do sistema (sem CDN); grade: cabeçalho (título + versão + métricas idle/RAM/flash/uptime com tooltips) + linha de estado (temperatura em destaque, gauge canvas semicircular 20–90 °C, estado/badges) + cartão do gráfico (canvas, 0–120 amostras, eixo Y fixo 20–90 °C com rótulos à esquerda, grade cinza claro a cada 10 °C, linha tracejada de limite 80 °C, série em azul) + controles (ON/OFF com cor de estado: verde ligado / vermelho desligado) + área de alertas (alarme ≥ 80 °C, falha de leitura, sensor ausente) + botão **Rearmar** visível com latch ativo.
- Tooltips em todos os elementos de controle e visualização (atributo `title` + ícone “?” nos cartões).
- Polling `fetch('/json')` a cada 2000 ms; sem recarga de página; limiar de reflexão ≤ 2,5 s (FR-029; DEC-07).
- Layout sem rolagem em 1366×768; media query ≤ 900px empilha colunas e mantém cartão do gráfico com `min-height: 320px` (correção de colapso de canvas em viewport estreito).
- Feedback de recusa mapeia `reason` → mensagem pt-BR (“bloqueado por alarme latcheado”, “sensor ausente”, “leitura inválida”, “temperatura ≥ 80 °C”).

### 4.6 `console` — protocolo de bancada (FR-030, somente `-DBENCH_TEMP_INJECTION=1`)

Linhas ASCII terminadas em `\n`; resposta sempre em uma linha `OK …`, `ERR …` ou `JSON …`. Comandos:

| Comando | Efeito | Resposta típica |
|---|---|---|
| `TEMP <centi>` | ativa injeção de leitura válida (ex.: `TEMP 8000` = 80,00 °C) | `OK TEMP 8000` |
| `FAULT` | ativa injeção de leitura inválida | `OK FAULT` |
| `REAL` | desativa injeção (volta ao sensor físico) | `OK REAL` |
| `ON` / `OFF` | comando de carga pela mesma política das rotas | `OK ON pwm=1023` / `ERR ON reason=latched` |
| `REARM` | rearme manual do latch | `OK REARM` / `ERR REARM reason=temp_high` |
| `STATE` | imprime o payload `/json` | `JSON {…}` |
| `RESCAN` | força varredura OneWire | `OK RESCAN sensor=1` |
| `REBOOT` | reinicia o MCU (evidência CA-018) | `OK REBOOT` |
| `HELP` | lista comandos | `OK HELP …` |

Injeção substitui `Reading` e presença **apenas** na amostragem; a política, o buzzer, o histórico e o JSON seguem os mesmos caminhos do sistema real (o teste exercita o comportamento, não um atalho).

### 4.7 `tools/bench_injection_test.py` (pyserial)

Bateria de proteção (≈ 10 verificações): sensor presente → `ON` com 79,90 °C mantém PWM 1023 sem alarme; 80,00 °C ⇒ PWM 0 + latch + alarme; `ON` recusado com latch; `REARM` recusado a 85 °C (`temp_high`); após esfriar a 50 °C o latch persiste; `REARM` aceito e carga continua desligada; novo `ON` efetiva; `FAULT` ⇒ PWM 0 + `invalid_reading` + `ON` recusado; leitura válida de retorno remove o bloqueio sem religar; alinhamento do `samp_min/max` à janela CA-004. Saída: veredito por item e código de saída ≠ 0 em falha.

## 5. Execução, concorrência e ownership

- **Contexto único**: todo o código roda no loop principal (`setup`/`loop`); callbacks do `ESP8266WebServer` são invocados por `handleClient()` no próprio loop. Não há ISR, task ou thread — por isso **não existem** variáveis compartilhadas com ISR/DMA, `volatile` de sincronização, seções críticas, locks ou filas; os buffers têm owner único (o loop).
- **Sem registradores manipulados diretamente**: GPIO/PWM via API do core (`pinMode`, `digitalWrite`, `analogWriteRange`, `analogWrite`); OneWire/DallasTemperature via bibliotecas. Nenhum read-modify-write de registrador pelo firmware.
- **Ordem de atuação**: comandos HTTP/console mutam a política de forma síncrona; a aplicação física (PWM/buzzer) ocorre no passo 6 do loop — mesma iteração, portanto o corte por alarme é efetivado na mesma avaliação em que é detectado.

### 5.1 Contabilidade de carga (`load_pct`/`idle_pct`)

Métrica documentada (não é medição de CPU do SDK): por janela de 1 s, `load = Σ micros()` dos blocos de trabalho do loop (amostragem + `handleClient()` + console) dividido pela janela; `idle = 100 − load` (clamp 0–100). Serve para evidenciar o custo do polling HTTP (CA-021) e tendências de degradação; o subjacente de rádio/SDK não é contabilizado.

## 6. Orçamento de tempo e memória

| Item | Valor/alvo | Observação |
|---|---|---|
| Amostragem | 1200 ms ± 150 ms (DEC-07) | `PeriodicTimer` + conversão assíncrona; sem dependência do HTTP |
| Conversão DS18B20 | 750 ms (12 bits) | sempre < período; nunca aguardada |
| Buzzer | 150 ms ON / 2000 ms OFF | ± 20% por fase (bancada, DEC-06) |
| Resposta HTTP | ≤ 500 ms sob alarme e polling ≥ 1 Hz | payload/buffer estático; HTML PROGMEM |
| Re-varredura OneWire | 5000 ms | só quando ausente há interesse: cadência fixa simples |
| Sketch | ≤ 45% de 1.044.464 B | medido a cada build; HTML em PROGMEM |
| RAM estática | ≤ 50% de 81.920 B | buffers: JSON 1600 B + trend 240 B + console 64 B + pilha 4 kB do SDK |
| Alocação dinâmica | nenhuma no loop | buffers estáticos; sem `String` em acúmulo |

## 7. Reset, watchdog, brownout e recuperação (FR-016/FR-017, CA-018)

- Boot/reset ⇒ estado seguro **por construção**: `begin()` desliga a carga (PWM 0), buzzer LOW, latch limpo (volátil, sem persistência) e bloqueio até a primeira leitura válida (DEC-02).
- Motivo do reset registrado no serial (`ESP.getResetReason()`, campo `reset` do JSON) quando disponível.
- Não bloqueio permanente do watchdog: nenhum laço interno espera; `handleClient()` é cooperativo; o loop sempre retorna.
- Após brownout/watchdog em operação: o latch é volátil e será perdido — comportamento esperado documentado (risco residual R7 do CODEBASE): a carga permanece desligada e o sistema volta a bloquear até leitura válida; a condição térmica ≥ 80 °C volta a latchear na primeira leitura válida.

## 8. Estratégia de verificação e bancada (resolve QA-4)

1. **HOST (gate mínimo)**: Unity em `firmware/test/` cobre fronteira 79,9/80,0 °C, latch/rearme, bloqueios, fail-safe, timer com wraparound, fases do buzzer, histórico circular, contrato JSON (campos/ordem/truncamento). Nenhuma ramificação de segurança sem teste.
2. **BANCADA — injeção (env `bancada`)**: bateria §4.7 exercita corte/latch/rearme/recusa **no firmware real**, com segurança elétrica (a carga pode ser deixada desconectada; a injeção simula o sensor e a temperatura). Cobre CA-008…CA-016, CA-024 e a evidência de intervalo de amostragem.
3. **BANCADA — física assistida (operador)**: desconectar/reconectar o D2 para CA-002/CA-003/CA-017 (sensor real), audição do buzzer para CA-013/CA-014, reset forçado para CA-018, associação Wi-Fi/DHCP/HTTP para CA-019/CA-020, polling sob alarme para CA-021, checklist visual do dashboard para CA-022/CA-023.
4. **Limitação registrada**: ensaio térmico real na fronteira de 80 °C **não** será exigido como evidência primária (risco elétrico/térmico de bancada); a fronteira é validada por HOST + injeção no firmware real. Se houver meio de referência futuramente, a evidência física pode ser acrescentada sem mudança de código.
5. Evidências em `docs/05-testing/` (logs, JSONs capturados, medições); `PASS` só com evidência; pendências físicas explícitas em `STATUS.md`/`HANDOFF.md`.

## 9. ADRs

**ADR-001: Camadas pura × plataforma** | Contexto: o requisito mais crítico (segurança térmica) precisa de teste determinístico em host; o SDK do ESP8266 não roda em HOST. | Decisão: FSM, timer, buzzer, tendência e JSON em `lib/thermal_logic` sem `Arduino.h`; integração mínima em `src`. | Consequências: gate HOST forte sobre a segurança; duplicação pequena de constantes mitigada por `static_assert` contra `config.h`; exige disciplina para não vazar API do SDK para a lib.

**ADR-002: Amostragem por tick de 1 s com conversão assíncrona** (cadência revista por ADR-013: 1200 ms) | Contexto: DS18B20 12 bits converte em ~750 ms; `delay()`/timers/ISR proibidos. | Decisão: `PeriodicTimer` de 1000 ms no loop; `setWaitForConversion(false)`; ler resultado anterior e re-disparar no mesmo tick (§4.4). | Consequências: conversão sempre pronta (1000 > 750); jitter limitado ao loop; nenhum bloqueio; a primeira leitura após o boot sai no 1º tick. Cadência revista para 1200 ms por ADR-013 (DEC-07). |

**ADR-003: Política de segurança como FSM pura com limpeza fail-safe do comando** | Contexto: FR-010/011/015 e DEC-04 exigem corte imediato, latch e nenhum religamento automático. | Decisão: `update()` avalia em ordem fixa (alarme → latch → bloqueio → limpa `heater_command` quando bloqueado); `pwm()` é derivado do estado, nunca comandado direto. | Consequências: impossível "esquecer" a carga ligada sob bloqueio; ON pós-desbloqueio é sempre explícito; comportamento 100% testável em HOST.

**ADR-004: Latch volátil com rearme manual duplo (dashboard + console)** | Contexto: sem persistência (proibida), reset limpa o latch; QA-1 perguntava o mecanismo de rearme. | Decisão: rearme por `/rearm` (botão) e `REARM` (console), aceite só com leitura válida < 80,0 °C; após rearme a carga fica desligada até novo ON (DEC-01). | Consequências: consistente com o hardware; estado pós-reset é seguro porém exige operação; risco residual R7 documentado.

**ADR-005: Contrato JSON em centésimos inteiros com buffer estático e truncamento seguro** | Contexto: RAM limitada, sem `float`, dashboard/console/telemetria compartilham o payload; QA-3/DEC-03 pedem contrato fixo. | Decisão: §3.5 — ordem fixa, `int16` centésimos, builder com `cap` e truncamento do `hist` quando necessário. | Consequências: parsing JS trivial (`/100`); nenhuma alocação; limite previsível (1600 B); `hist_count` sempre reflete o emitido.

**ADR-006: Comandos HTTP síncronos no mesmo contexto do loop** | Contexto: callbacks do `ESP8266WebServer` já rodam em `handleClient()` (loop); não há ISR. | Decisão: handlers chamam a política diretamente e respondem JSON com `ok/reason/pwm/latch/state`; atuação física no passo de atuação do loop. | Consequências: ordenação determinística entre comando e amostra; sem fila/lock; feedback imediato ao operador.

**ADR-007: Console de bancada em tempo de compilação** | Contexto: FR-030 exige ferramenta que **não** exista em produção. | Decisão: `console` e o caminho de injeção atrás de `BENCH_TEMP_INJECTION`; env `bancada` com flag, env `nodemcuv2` sem. | Consequências: binário de produção sem superfície de teste; evidência HOST compara compilação dos dois envs (CA-024).

**ADR-008: Versões fixadas (resolve QA-5)** | Contexto: NFR-005 exige reprodutibilidade. | Decisão: `platform = espressif8266@4.2.1` (framework Arduino Core 3.30102.0), `OneWire@2.3.8`, `DallasTemperature@3.11.0`; toolchain do PlatformIO. | Consequências: build reprodutível; atualização exige nova decisão e reverificação; documentado no `platformio.ini`.

**ADR-009: Métrica de carga/idle por contabilidade dos blocos do loop** | Contexto: descricao §5 pede métricas de saúde; o SDK não expõe carga de CPU utilizável. | Decisão: §5.1 — proxy documentado (tempo em amostragem+HTTP+console por janela de 1 s). | Consequências: mede o que o firmware controla (custo do polling); não substitui medição de CPU; rótulo e documentação explícitos.

**ADR-010: UI estática em PROGMEM com tooltips nativos e polling de 1 s** (polling revisto por ADR-013: 2000 ms) | Contexto: AP sem internet; FR-021…FR-027/FR-029; orçamento de flash. | Decisão: HTML/CSS/JS únicos embutidos; `title`/“?” para tooltips; sem frameworks; `fetch('/json')` a cada 1 s; layout 1366×768 sem rolagem com fallback ≤ 900px. | Consequências: zero dependência externa; footprint previsível; correção de colapso de canvas mantida no CSS responsivo. Polling revisto para 2000 ms por ADR-013 (DEC-07). |

**ADR-011: Gestão Wi-Fi sem gravação em flash** | Contexto: AP fixo com IP fixo; persistência proibida; NFR-004 exige ≤ 500 ms. | Decisão: `WiFi.persistent(false)` + `setSleepMode(WIFI_NONE_SLEEP)` + `softAPConfig` + `softAP` aberto com SSID derivado do MAC (DEC-05, ex.: `ESP8266_1A2B3C`). | Consequências: sem writes de SDK na flash; SSID muda por placa (documentado para bancada); DHCP padrão do softAP atende 192.168.4.0/24; consumo de rádio maior (irrelevante na alimentação por USB), eliminando o atraso de power-save (~3 s) medido na primeira `/` após ociosidade.

**ADR-012: lwIP com MSS 1460 (variante higher bandwidth)** | Contexto: o default do core (`PIO_FRAMEWORK_ARDUINO_LWIP2_LOW_MEMORY`: `TCP_MSS=536`, `TCP_SND_BUF≈1 KB`, 5 PCBs) engasava o envio da página de ~12,9 KB a clientes lentos: `/` chegava a ~3 s, houve **truncamento** (conexão fechada com 955–11,5 KB de 12,9 KB) e a amostragem suspendeu (janela 197/1803 ms) — evidência de bancada 2026-09-12. | Decisão: `-DPIO_FRAMEWORK_ARDUINO_LWIP2_HIGHER_BANDWIDTH` (`TCP_MSS=1460`, `TCP_SND_BUF≈2,9 KB`), mantendo `WiFi.setSleepMode(WIFI_NONE_SLEEP)`. | Consequências: transferências ~3× mais eficientes e menos rodadas de backpressure; RAM de pools lwIP maior (medida no build, dentro da meta da constituição §3); páginas grandes deixam de bloquear a amostragem; clientes com power-save agressivo ainda podem atrasar ACKs (documentado como limitação de cliente, mitigada por keep-alive como o usado pela dashboard).

**ADR-013: Vínculo à ROM do DS18B20 e cadências revisadas (amostragem 1200 ms; dashboard 2000 ms — DEC-07)** | Contexto: decisão do proprietário (2026-09-12) de localizar a ROM na inicialização, usá-la nas leituras, amostrar a cada 1,2 s e atualizar a dashboard a cada 2 s; F1 §4.2 previa 1 s ± 150 ms; a contagem de dispositivos do `DallasTemperature` é cacheada (`getDeviceCount()` não re-varre), o que tornava a re-varredura de FR-003 inoperante em runtime. | Decisão: varredura no `setup` enumera o barramento e vincula a ROM (`getAddress`); conversão por `requestTemperaturesByAddress(rom)` e leitura por `getTempC(rom)`; re-varredura de 5 s chama `begin()` (re-enumeração real) e revincula a ROM; `kSamplePeriodMs = 1200` (conversão de 750 ms segue < período); dashboard com `fetch('/json')` a cada 2000 ms e reflexão ≤ 2,5 s; janela CA-004 passa a [1050 ms, 1350 ms] e o histórico de 120 amostras cobre 144 s. | Consequências: desvio da F1 registrado (DEC-07); detecção de conexão/substituição do sensor em runtime passa a funcionar (FR-003); orçamento de reflexão da UI revisado; ferramenta de bancada e evidências atualizadas.

### 9.1 Alternativas rejeitadas

| Alternativa | Motivo da rejeição |
|---|---|
| FSM de segurança direto em `main.cpp` | Não testável em HOST; violaria o gate mínimo e a constituição §5. |
| `delay(750)` ou espera por `isConversionComplete()` no mesmo tick | Bloqueio proibido (NFR-001); arriscaria watchdog e jitter. |
| Timer de hardware/ISR para amostragem | Proibido por `AGENTS.md` §5 e pela constituição §2. |
| Latch persistido em EEPROM/SPIFFS | Persistência proibida; DEC-02 fixa latch volátil e estado seguro pós-reset. |
| Relé/buzzer por PWM em GPIO16 | GPIO16 não suporta PWM/IRQ; apenas digital. |
| Parser JSON no ESP (para config remota) | Fora de escopo; aumenta superfície e RAM sem requisito. |
| Dashboard com página inteira em refresh | Overhead e UX pior; FR-029 exige AJAX sem recarga. |
| Adiar corte de segurança até o fim da resposta HTTP | Violaria "mesma avaliação" (FR-010); a avaliação precede a atuação no loop. |

## 10. Riscos residuais do design

- **R7/CODEBASE (latch volátil)**: reset em condição de alarme limpa o latch; mitigado pelo estado seguro + novo latch na primeira leitura ≥ 80 °C; aceito e documentado.
- **Proxy de carga (§5.1)**: não mede o custo de rádio/SDK; aceito como métrica de saúde do firmware.
- **Pull-up OneWire (P3)**: fora do controle do firmware; a varredura na bancada confirma o hardware.
- **Buzzer audível**: ciclo verificado por lógica (HOST) e injeção; audição depende do operador.

## Referências

- `spec.md` (feature `controle-termico`) — requisitos e critérios `CA-###`.
- `docs/descricao.txt` — fonte normativa (não editar).
- `.specs/project/constitution.md` — princípios e metas de recursos.
- `.specs/project/ROADMAP.md` — escopo, riscos, marcos e QA-4/QA-5.
- `.specs/codebase/CODEBASE.md` — stack, convenções, concerns.
- `AGENTS.md` — restrições de hardware, convenções e comandos.
- `.github/skills/sdd-embarcado/SKILL.md` — formato de design e critérios.
