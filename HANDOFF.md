# HANDOFF — 2026-09-12 → próxima sessão

## Contexto

Reimplementação completa do firmware do **Sistema de Monitoramento e Controle Térmico** (`docs/descricao.txt`) concluída nesta sessão, do zero, seguindo o fluxo SDD da skill `sdd-embarcado`: `spec.md` → `design.md` → `tasks.md` → implementação → testes HOST → bancada com hardware real.

## Estado atual

- **Placa**: NodeMCU v2 em `/dev/ttyUSB0`, com o build de **produção** (`nodemcuv2`, sem console de bancada) gravado e rodando.
- **Rede**: AP aberto `ESP8266_101026` em 192.168.4.1/24 (DHCP ativo); dashboard e `/json` operacionais.
- **Estado de segurança**: DS18B20 **ausente** no momento → `NO_SENSOR`, carga bloqueada (PWM 0), latch limpo, buzzer silencioso — comportamento seguro esperado.
- **Host de desenvolvimento**: foi desconectado do AP ao final; para retestar: `nmcli dev wifi rescan && nmcli con up ESP8266_101026` (a conexão tem `wifi.powersave=2`). Voltar à rede doméstica: `nmcli con up linus`.

## Alterações realizadas (principais arquivos)

- `firmware/platformio.ini` — envs `nodemcuv2` (default), `bancada` (`-DBENCH_TEMP_INJECTION=1`), `native`; versões fixadas; **lwIP 1460 (ADR-012)**.
- `firmware/lib/thermal_logic/` — lógica pura: `control_policy` (FSM fail-safe: corte ≥ 80,00 °C, latch, rearme manual, bloqueios), `periodic_timer` (deadline wrap-safe), `buzzer_pattern` (150/2000 ms), `trend_buffer`, `status_json`.
- `firmware/src/` — `main.cpp` (loop: amostragem 1 s → política → atuação; Wi-Fi AP; métricas load/idle), `config.h` (pinos/tempos/limiares com `static_assert` contra a lib), `ds18b20_sensor` (conversão assíncrona), `web_server` (`/`, `/json`, `/on`, `/off`, `/rearm`), `web/dashboard_html.h` (dashboard pt-BR em PROGMEM, gauge + gráfico 20–90 °C, alertas, latch+rearme, tooltips), `console` (injeção — só `bancada`).
- `firmware/test/` — 5 suítes Unity (37 testes), `firmware/tools/bench_injection_test.py` (bateria 14 verificações).
- `.specs/features/controle-termico/` — `spec.md` (FR/NFR/CA + matriz), `design.md` (ADRs 001–012), `tasks.md` (T-001…T-017).
- `docs/05-testing/controle-termico/` — evidências: testes HOST, footprint, bateria, rede (keep-alive e churn), boot de produção, screenshots, checklist.

## Decisões

- **ADR-011**: `WIFI_NONE_SLEEP` (elimina atraso de ~3 s pós-ociosidade).
- **ADR-012**: lwIP MSS 1460 (corrige truncamento da página de 12,9 KB e suspensões da amostragem sob carga; antes: 197/1803 ms de jitter, página truncada).
- Jitter em regime exposto em `samp_win_min_ms`/`samp_win_max_ms` (janela de 60 intervalos).
- Latch volátil + rearme manual (dashboard e console); após rearme/retomada, novo `ON` obrigatório (DEC-01/DEC-04).
- Console de bancada em tempo de compilação (inexistente em produção).

## Problemas resolvidos na sessão

1. `PeriodicTimer` disparava duas vezes no mesmo `millis()`/estourava com deadline no futuro → reescrito com deadline por diferença sinalizada (coberto por teste HOST de wraparound).
2. `/` em ~3 s e **truncamento da página** (955 B–11,5 KB de 12.876 B) + amostragem suspensa sob carga → lwIP 1460 + sleep off (ADR-011/012); pós-correção: páginas em 31–81 ms completas, `/json` máx 26,6 ms.
3. Estatística de jitter poluída pelo transiente de boot → janela de 60 intervalos.

## Testes executados (evidências)

| Teste | Resultado | Arquivo |
|---|---|---|
| HOST (`pio test -d firmware -e native`) | 37/37 PASS | `2026-09-12-host-tests.log` |
| Bateria de proteção (injeção, hardware real) | 14/14 PASS | `2026-09-12-bancada-bateria-injecao.log` |
| Rede keep-alive sob alarme (60 s) | PASS (`/json` máx 26,6 ms; janela 999–1001 ms) | `2026-09-12-bancada-rede-keepalive.log` |
| Rede churn (conexão nova por requisição) | PASS (máx 38,6 ms; 0 acima de 500 ms) | `2026-09-12-bancada-rede-churn.log` |
| Boot de produção | AP + HTTP ok; console ausente | `2026-09-12-boot-producao.log` |
| Dashboard 1366×768 (sem rolagem, 23 tooltips) | PASS | `2026-09-12-dashboard-*.png` |
| Footprint (produção/bancada) | 31,2%/39,1% · 31,4%/41,6% | `2026-09-12-build-footprint.log` |

## Pendências (ação do operador)

1. **CA-001/CA-003** — conectar o DS18B20 no D2/GPIO4 com o sistema rodando: esperar `sensor=true`, ROM logada (`[sensor] DS18B20 detectado (ROM …)`), leituras a cada 1 s e histórico no gráfico. Desconectar e religar → re-varredura ≤ 5 s e retomada.
2. **CA-015 (variante física)** — com leitura válida e carga ligada, desconectar o sensor: PWM 0, `block` explícito, alerta na dashboard; reconectar: bloqueio removido, **sem religamento automático**.
3. **CA-013 (audição)** — regravar `bancada` (`pio run -d firmware -e bancada -t upload --upload-port /dev/ttyUSB0`), enviar `TEMP 8000` (`python3 -m serial.tools.miniterm /dev/ttyUSB0 115200` ou script) e confirmar o bipe de ~150 ms a cada ~2 s.
4. (Opcional) ensaio térmico real 79,9/80,0 °C — QA-4 permite manter injeção como evidência primária.

## Próximo passo

Executar os itens 1–3 acima, registrar resultados no checklist (`docs/05-testing/controle-termico/2026-09-12-checklist-bancada.md`) e atualizar a matriz (`spec.md` §9) e este handoff.

## Cuidados

- Comandos: `-d` pertence ao subcomando (`pio run -d firmware`, `pio test -d firmware -e native`); porta `/dev/ttyUSB0` @ 115200.
- Abrir a serial pode **reiniciar a placa** (motivo registrado como `External System`) e limpar a injeção (volátil) — normal; aguardar o boot antes de enviar comandos.
- Injeção só existe no env `bancada`; para voltar ao estado de entrega: gravar `nodemcuv2`.
- `nmcli` engana com cache de scan — usar `--rescan yes`/`nmcli dev wifi rescan` antes de conectar.
- Não fazer commit automático; alterações de comportamento exigem atualização de `spec.md`/`design.md` antes.

## Critério de conclusão

Itens 1–3 das pendências executados e registrados com evidência; matriz de rastreabilidade sem `PENDENTE` (exceto ensaio térmico opcional, explicitamente dispensado por QA-4).
