# Checklist de validação de bancada — `controle-termico` (2026-09-12)

| Campo | Valor |
|---|---|
| Firmware | `1.0.0` — reimplementação completa (fluxo SDD: `spec.md` → `design.md` → `tasks.md`) |
| Alvo | NodeMCU v2 (ESP8266MOD), MAC 80:7D:3A:10:10:26 → AP `ESP8266_101026` |
| Porta serial | `/dev/ttyUSB0` (115200) |
| Envs usados | `nodemcuv2` (produção, gravado ao final), `bancada` (injeção serial), `native` (testes HOST) |
| Host de teste | Linux (NetworkManager), DHCP 192.168.4.100/24, power-save **desligado** na conexão com o AP (`wifi.powersave=2`) |
| Métodos | Bateria automatizada (`firmware/tools/bench_injection_test.py`), sessões HTTP instrumentadas, dashboard real em navegador (1366×768) |

Legenda: **PASS** = evidência registrada · **PASS\*** = evidência com nota/limitação · **PENDENTE** = requer ação física do operador (indisponível em 2026-09-12).

> **Revisão DEC-07 (2ª sessão de bancada, 2026-09-12):** amostragem **1200 ms ± 150 ms** e polling da dashboard **2000 ms** (reflexão ≤ 2,5 s); leitura/conversão do DS18B20 **vinculadas à ROM** identificada na varredura; re-varredura corrigida (`getDeviceCount()` do `DallasTemperature` é cacheado — `rescan()` agora chama `begin()`). Evidências novas com sufixo `-dec07`.

## Resultados por critério de aceitação

| CA | Resultado | Evidência |
|---|---|---|
| CA-001 | PASS* | ROM **vinculada e usada nas leituras** (`[sensor] DS18B20 detectado (ROM 28FFE203B41605C2)`; leituras válidas 36–37 °C) — `...-bateria-injecao-dec07.log`, `...-rede-dec07.log`; limitação de **hardware**: no boot deste protótipo a detecção falhou de forma intermitente (vínculo na re-varredura, 5–15 s) e o barramento alterna presente/ausente — sintoma de pull-up ausente/fraco (P3); carga sempre bloqueada sem leitura válida |
| CA-002 | PASS* | Ausência **real** observada: `state=NO_SENSOR`, `sensor=false`, carga bloqueada e `ON` recusado com motivo (`2026-09-12-bancada-rede.log`); re-varredura de 5 s implementada (timer coberto em `test_periodic_timer`) |
| CA-003 | PASS | Detecção e perda em runtime observadas com o sensor físico (mesma ROM revinculada): `[sensor] DS18B20 detectado (ROM …)` + `sensor detectado em runtime` e `[sensor] sensor ausente — carga bloqueada` — `2026-09-12-bancada-sensor-instavel-dec07.log`, `2026-09-12-boot-dec07.log` |
| CA-004 | PASS | HOST (`test_periodic_timer`) + bancada (DEC-07): janela [1050, 1350] ms — bateria `min=1176 max=1224`; `samp_min/max/win = 1200` sob polling de 60 s (`2026-09-12-bancada-bateria-injecao-dec07.log`, `2026-09-12-bancada-rede-dec07.log`) |
| CA-005 | PASS | Conversão assíncrona (`setWaitForConversion(false)`; leitura ao fim do ciclo e re-disparo imediato — inspeção) + conversão/leitura da **ROM vinculada** (`requestTemperaturesByAddress`/`getTempC`); cadência mantida sob alarme/polling (`...-rede-dec07.log`) |
| CA-006 | PASS | HOST (`test_control_policy`) + bancada via HTTP: `/on` → PWM 1023, `/off` → 0 (`2026-09-12-bancada-rede-keepalive.log`); console ON/OFF (`...-bateria-injecao.log`) |
| CA-007 | PASS | HOST + bancada: recusas com motivo (`no_sensor`/`invalid_reading`/`latched`) e PWM mantido em 0 (bateria + `...-rede.log`) |
| CA-008 | PASS | HOST (fronteira 79,99/80,00) + bateria: 79,90 °C mantém PWM 1023 sem alarme |
| CA-009 | PASS | HOST + bateria checks 4 e 10: 80,00 °C → PWM 0 + latch + alarme na mesma avaliação (com carga ligada) |
| CA-010 | PASS | Bateria check 7: latch persiste a 50,00 °C; alarme cessa (buzzer silencia) |
| CA-011 | PASS | Bateria check 8 + `GET /rearm` aceito com leitura válida < 80 °C; carga segue desligada (`...-rede-keepalive.log`) |
| CA-012 | PASS | Bateria checks 5, 6 e 9: rearme recusado com `latched`/`temp_high` e latch mantido |
| CA-013 | PASS (HOST); audição PENDENTE | Fases 150/2000 ms verificadas nos testes HOST e acionamento observado na bancada (injeção); confirmação **audível** depende do operador |
| CA-014 | PASS | HOST + injeção `FAULT`: sem condição ≥ 80 °C ⇒ `alarm=false` (buzzer permanece desligado) |
| CA-015 | PASS* | HOST + injeção `FAULT` (PWM 0, `block=invalid_reading`, `ON` recusado — bateria check 11) + **perda física observada** (barramento instável → `NO_SENSOR`/`INVALID_READING` com PWM 0 e estado explícito — `...-sensor-instavel-dec07.log`); corte a partir de carga ligada evidenciado por injeção (checks 4/10) |
| CA-016 | PASS | Bateria check 12: bloqueio removido com leitura válida, **sem religamento** (DEC-04); novo `ON` aceito |
| CA-017 | PASS | Sensor ausente real e falha injetada expostos explicitamente na dashboard e no `/json`, sem temperatura falsa (`temp:"—"`, `2026-09-12-dashboard-producao-sensor-ausente.png`) |
| CA-018 | PASS | Bateria check 14: reset limpa o latch (volátil) e mantém carga desligada; boot de produção com motivo registrado (`reset: External System`) |
| CA-019 | PASS | AP aberto `ESP8266_101026` associado por cliente real; DHCP 192.168.4.100/24, gateway 192.168.4.1; dashboard servida (resumo em `...-rede.log`) |
| CA-020 | PASS | Payload real com todos os campos do contrato (26+2 campos, ordem fixa) + HOST (`test_status_json`); capturas em `...-rede*.log` |
| CA-021 | PASS | 2ª sessão (DEC-07): 59 requisições em 60 s sob alarme, média **13 ms**, máx **17 ms**; página de 12.913 B em **32 ms**; janela de amostragem 1200 ms; sem reset (`2026-09-12-bancada-rede-dec07.log`). 1ª sessão: keep-alive máx 26,6 ms; churn máx 38,6 ms (`...-rede-keepalive.log`, `...-rede-churn.log`) |
| CA-022 | PASS | Navegador real 1366×768: `scrollHeight=768` (**sem rolagem**), **23 tooltips**, valor numérico + gauge, gráfico 20–90 °C com grade e limite 80 °C, botão com cores de estado, métricas (idle/RAM/flash/uptime), alertas e latch+rearme — screenshots `2026-09-12-dashboard-*.png` |
| CA-023 | PASS | Polling medido por Resource Timing: **2000 ms** entre `fetch('/json')`; reflexão no DOM **1426 ms** (ON) e **1996 ms** (OFF), ambos ≤ 2,5 s (DEC-07), sem recarga de página (`2026-09-12-dashboard-dec07.log`, `2026-09-12-dashboard-dec07.png`) |
| CA-024 | PASS | `strings`: console presente no env `bancada` (1) e ausente no `nodemcuv2` (0); bateria usa os comandos no hardware real |
| CA-025 | PASS | Build: sketch 31,2% e RAM 39,1% (produção); 31,4%/41,6% (bancada) — metas ≤ 45% e ≤ 50% (`2026-09-12-build-footprint.log`) |
| CA-026 | PASS | Inspeção (sem `delay()`, sem ISR, sem timers de hardware, sem persistência) + sessões contínuas de bancada sem reset espontâneo |
| CA-027 | PASS* | Versões fixadas (`espressif8266@4.2.1`, framework 3.30102.0, OneWire 2.3.8, DallasTemperature 3.11.0); "checkout limpo" aproximado por rebuild completo (`-t clean` + build) |
| CA-028 | PASS | Matriz de rastreabilidade atualizada (`spec.md` §9) + evidências em `docs/05-testing/controle-termico/` |

## Achados e correções durante a bancada (registrados no design)

1. **Amostragem suspensa ao enviar a página** (janela 197/1803 ms) e **truncamento da página** (955 B–11,5 KB de 12.876 B) com o lwIP default (`TCP_MSS=536`, buffer ~1 KB, 5 PCBs) → corrigido com `PIO_FRAMEWORK_ARDUINO_LWIP2_HIGHER_BANDWIDTH` (**ADR-012**); antes: `/` levava ~3 s e fechava conexões.
2. **Atraso de ~3 s na primeira `/`** após ociosidade (power-save do rádio) → `WiFi.setSleepMode(WIFI_NONE_SLEEP)` (**ADR-011**).
3. **Jitter poluído pelo transiente de boot nas estatísticas cumulativas** → campos `samp_win_min_ms`/`samp_win_max_ms` (janela dos últimos ≤60 intervalos) no contrato `/json`, permitindo evidência de regime (CA-004/CA-021).
4. Power-save do cliente (host Linux) agrava atrasos de ACK; nos testes foi desligado na conexão. Firmware agora tolera clientes lentos (sem truncamento com MSS 1460).
5. **Re-varredura ineficaz (2ª sessão):** `getDeviceCount()` do `DallasTemperature` é **cacheado** e não re-varre o barramento — a re-varredura de 5 s (FR-003) nunca detectaria conexão/desconexão em runtime. Corrigido: `rescan()` chama `begin()` (re-enumeração real) e revincula a ROM (**ADR-013**).
6. **Barramento OneWire intermitente neste protótipo:** a mesma ROM alternou presente/ausente em rescans sucessivos e a detecção de boot falhou em todos os resets observados (recuperação automática pela re-varredura). Sintoma compatível com pull-up ausente/fraco — **P3 segue em aberto** (confirmar 4,7 kΩ entre D2 e 3,3 V). O firmware permanece seguro em todas as transições (carga bloqueada; estado explícito no `/json` e na dashboard).

## Limitações e pendências físicas

- **CA-001** mantém limitação de hardware (detecção de boot intermitente) e **P3** (pull-up OneWire) segue **em aberto**: confirmar 4,7 kΩ entre DQ (D2) e 3,3 V e a fixação dos fios — sem isso a detecção/reconexão fica intermitente, embora o firmware bloqueie e recupere sozinho (FR-002/FR-003).
- **CA-013 audição do buzzer**: depende do operador (gravar `bancada`, `TEMP 8000`, ouvir 150 ms a cada ~2 s); lógica já verificada em HOST e por acionamento em injeção.
- **Ensaio térmico real** na fronteira de 80 °C não é evidência primária exigida (QA-4): fronteira validada por HOST + injeção no firmware real.
- Cliente pode ocasionalmente fechar a conexão keep-alive ociosa (comportamento do core HTTP); navegadores reconectam de forma transparente (1 reconexão em 60 requisições na sessão medida).

## Como reproduzir

```bash
pio test -d firmware -e native                                      # gate HOST (37 testes)
pio run -d firmware                                                 # build de produção
pio run -d firmware -e bancada -t upload --upload-port /dev/ttyUSB0 # bancada (injeção)
python3 firmware/tools/bench_injection_test.py --port /dev/ttyUSB0  # bateria de proteção
pio run -d firmware -t upload --upload-port /dev/ttyUSB0            # produção (estado final)
```
