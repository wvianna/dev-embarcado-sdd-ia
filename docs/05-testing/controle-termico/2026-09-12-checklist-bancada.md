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

## Resultados por critério de aceitação

| CA | Resultado | Evidência |
|---|---|---|
| CA-001 | PENDENTE (detecção física) | Ausência registrada no boot (`[sensor] nenhum DS18B20 detectado` em `2026-09-12-boot-producao.log`); identificação de ROM requer sensor conectado ao D2 |
| CA-002 | PASS* | Ausência **real** observada: `state=NO_SENSOR`, `sensor=false`, carga bloqueada e `ON` recusado com motivo (`2026-09-12-bancada-rede.log`); re-varredura de 5 s implementada (timer coberto em `test_periodic_timer`) |
| CA-003 | PENDENTE | Requer conectar o DS18B20 com o sistema em operação (re-varredura + retomada de leituras) |
| CA-004 | PASS | HOST (`test_periodic_timer`) + bancada: janela dos últimos 60 intervalos = 999–1001 ms sob polling e alarme; bateria `min=max=1000` (`2026-09-12-bancada-rede-keepalive.log`, `2026-09-12-bancada-bateria-injecao.log`) |
| CA-005 | PASS | Conversão assíncrona (`setWaitForConversion(false)`, leitura ao fim do ciclo e re-disparo imediato — inspeção) + janelas de amostragem estáveis durante alarme/polling |
| CA-006 | PASS | HOST (`test_control_policy`) + bancada via HTTP: `/on` → PWM 1023, `/off` → 0 (`2026-09-12-bancada-rede-keepalive.log`); console ON/OFF (`...-bateria-injecao.log`) |
| CA-007 | PASS | HOST + bancada: recusas com motivo (`no_sensor`/`invalid_reading`/`latched`) e PWM mantido em 0 (bateria + `...-rede.log`) |
| CA-008 | PASS | HOST (fronteira 79,99/80,00) + bateria: 79,90 °C mantém PWM 1023 sem alarme |
| CA-009 | PASS | HOST + bateria checks 4 e 10: 80,00 °C → PWM 0 + latch + alarme na mesma avaliação (com carga ligada) |
| CA-010 | PASS | Bateria check 7: latch persiste a 50,00 °C; alarme cessa (buzzer silencia) |
| CA-011 | PASS | Bateria check 8 + `GET /rearm` aceito com leitura válida < 80 °C; carga segue desligada (`...-rede-keepalive.log`) |
| CA-012 | PASS | Bateria checks 5, 6 e 9: rearme recusado com `latched`/`temp_high` e latch mantido |
| CA-013 | PASS (HOST); audição PENDENTE | Fases 150/2000 ms verificadas nos testes HOST e acionamento observado na bancada (injeção); confirmação **audível** depende do operador |
| CA-014 | PASS | HOST + injeção `FAULT`: sem condição ≥ 80 °C ⇒ `alarm=false` (buzzer permanece desligado) |
| CA-015 | PASS* | HOST + injeção `FAULT`: PWM 0 imediato, `block=invalid_reading`, `ON` recusado (bateria check 11); variante com desconexão física real pendente de operador |
| CA-016 | PASS | Bateria check 12: bloqueio removido com leitura válida, **sem religamento** (DEC-04); novo `ON` aceito |
| CA-017 | PASS | Sensor ausente real e falha injetada expostos explicitamente na dashboard e no `/json`, sem temperatura falsa (`temp:"—"`, `2026-09-12-dashboard-producao-sensor-ausente.png`) |
| CA-018 | PASS | Bateria check 14: reset limpa o latch (volátil) e mantém carga desligada; boot de produção com motivo registrado (`reset: External System`) |
| CA-019 | PASS | AP aberto `ESP8266_101026` associado por cliente real; DHCP 192.168.4.100/24, gateway 192.168.4.1; dashboard servida (resumo em `...-rede.log`) |
| CA-020 | PASS | Payload real com todos os campos do contrato (26+2 campos, ordem fixa) + HOST (`test_status_json`); capturas em `...-rede*.log` |
| CA-021 | PASS | Keep-alive ~60 s sob alarme: `/json` máx **26,6 ms** (média 12 ms), páginas 31–81 ms completas, janela 999–1001 ms, sem reset (`...-rede-keepalive.log`); cenário "conexão nova por requisição" (curl): 60/60, máx **38,6 ms** (`...-rede-churn.log`) |
| CA-022 | PASS | Navegador real 1366×768: `scrollHeight=768` (**sem rolagem**), **23 tooltips**, valor numérico + gauge, gráfico 20–90 °C com grade e limite 80 °C, botão com cores de estado, métricas (idle/RAM/flash/uptime), alertas e latch+rearme — screenshots `2026-09-12-dashboard-*.png` |
| CA-023 | PASS | Sem recarga: marcador de sessão do navegador preservado entre as transições normal→alarme→rearme→carga ligada→corte (≤ 2 s; verificações via DOM + screenshots) |
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

## Limitações e pendências físicas

- **CA-001/CA-003** (detecção/reconexão do DS18B20) e **CA-013 audição** e **CA-015 variante física**: dependem do operador (conectar/desconectar o sensor no D2; ouvir o buzzer). Roteiro pronto no `HANDOFF.md`.
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
