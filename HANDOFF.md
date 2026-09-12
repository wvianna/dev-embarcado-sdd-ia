# HANDOFF — 2026-09-12 (2ª sessão: revisão DEC-07) → próxima sessão

## Contexto

Reimplementação completa do firmware do **Sistema de Monitoramento e Controle Térmico** (`docs/descricao.txt`) concluída nesta sessão, do zero, seguindo o fluxo SDD da skill `sdd-embarcado`: `spec.md` → `design.md` → `tasks.md` → implementação → testes HOST → bancada com hardware real.

2ª sessão (mesmo dia): revisão **DEC-07** — vínculo da ROM do DS18B20 nas conversões/leituras, amostragem de 1200 ms e dashboard com polling de 2000 ms; revalidação de bancada (bateria, rede, dashboard) e produção regravada.

## Estado atual

- **Placa**: NodeMCU v2 em `/dev/ttyUSB0` com o build de **produção** (`nodemcuv2`, sem console) regravado e rodando (estado final).
- **Rede**: AP aberto `ESP8266_101026` em 192.168.4.1/24 (DHCP ativo); dashboard e `/json` operacionais (smoke final: `fw=1.0.0`, `hist_period=1200`, `samp_ms=1200`).
- **Estado de segurança no fechamento**: sensor físico **instável/ausente** → `NO_SENSOR`/`INVALID_READING`, carga bloqueada (PWM 0), latch limpo — comportamento seguro esperado (pendência de hardware: pull-up P3).
- **Host de desenvolvimento**: devolvido à rede doméstica (`linus`) ao final; para retestar: `nmcli dev wifi rescan && nmcli con up ESP8266_101026` (a conexão tem `wifi.powersave=2`).

## Alterações realizadas (principais arquivos)

- `firmware/platformio.ini` — envs `nodemcuv2` (default), `bancada` (`-DBENCH_TEMP_INJECTION=1`), `native`; versões fixadas; **lwIP 1460 (ADR-012)**.
- `firmware/lib/thermal_logic/` — lógica pura: `control_policy` (FSM fail-safe: corte ≥ 80,00 °C, latch, rearme manual, bloqueios), `periodic_timer` (deadline wrap-safe), `buzzer_pattern` (150/2000 ms), `trend_buffer`, `status_json`.
- `firmware/src/` — `main.cpp` (loop: amostragem 1,2 s → política → atuação; Wi-Fi AP; métricas load/idle), `config.h` (pinos/tempos/limiares com `static_assert` contra a lib), `ds18b20_sensor` (conversão assíncrona), `web_server` (`/`, `/json`, `/on`, `/off`, `/rearm`), `web/dashboard_html.h` (dashboard pt-BR em PROGMEM, gauge + gráfico 20–90 °C, alertas, latch+rearme, tooltips), `console` (injeção — só `bancada`).
- `firmware/src/ds18b20_sensor.{h,cpp}` (DEC-07) — varredura do boot vincula a **ROM** (`getAddress`); conversão `requestTemperaturesByAddress(rom)` e leitura `getTempC(rom)`; `rescan()` re-enumera com `begin()` (o `getDeviceCount()` do DallasTemperature é **cacheado** e não re-varre — bug corrigido).
- `firmware/src/config.h` — `kSamplePeriodMs = 1200`; `firmware/src/web/dashboard_html.h` — `setInterval(refresh, 2000)` + tooltip 1,2 s; `firmware/tools/bench_injection_test.py` — janela [1050, 1350] ms e `settle` 1,5 s.
- `.specs/features/controle-termico/` — `spec.md` DEC-07 (CA-004/CA-023/NFR-003 revisados), `design.md` ADR-013 + §3.2/§3.4/§4.1/§4.3/§4.4/§4.5.1/§6, `tasks.md` M5 (T-018…T-021); `constitution.md`, `AGENTS.md`, `ROADMAP.md`, `CODEBASE.md` e `README.md` alinhados.
- `docs/05-testing/controle-termico/` — evidências `2026-09-12-*-dec07.log`, `2026-09-12-dashboard-dec07.png`; checklist atualizado (CA-001/003/004/005/015/021/023).
- `firmware/test/` — 5 suítes Unity (37 testes), `firmware/tools/bench_injection_test.py` (bateria 14 verificações).
- `.specs/features/controle-termico/` — `spec.md` (FR/NFR/CA + matriz), `design.md` (ADRs 001–012), `tasks.md` (T-001…T-017).
- `docs/05-testing/controle-termico/` — evidências: testes HOST, footprint, bateria, rede (keep-alive e churn), boot de produção, screenshots, checklist.

## Decisões

- **ADR-011**: `WIFI_NONE_SLEEP` (elimina atraso de ~3 s pós-ociosidade).
- **ADR-012**: lwIP MSS 1460 (corrige truncamento da página de 12,9 KB e suspensões da amostragem sob carga; antes: 197/1803 ms de jitter, página truncada).
- Jitter em regime exposto em `samp_win_min_ms`/`samp_win_max_ms` (janela de 60 intervalos).
- Latch volátil + rearme manual (dashboard e console); após rearme/retomada, novo `ON` obrigatório (DEC-01/DEC-04).
- Console de bancada em tempo de compilação (inexistente em produção).
- **DEC-07/ADR-013**: amostragem **1200 ms ± 150 ms** (desvio intencional da F1 — decisão do proprietário) e dashboard com **polling de 2000 ms** (reflexão ≤ 2,5 s); conversões/leituras vinculadas à **ROM** identificada na varredura, com re-varredura de 5 s re-enumerando o barramento.

## Problemas resolvidos na sessão

1. `PeriodicTimer` disparava duas vezes no mesmo `millis()`/estourava com deadline no futuro → reescrito com deadline por diferença sinalizada (coberto por teste HOST de wraparound).
2. `/` em ~3 s e **truncamento da página** (955 B–11,5 KB de 12.876 B) + amostragem suspensa sob carga → lwIP 1460 + sleep off (ADR-011/012); pós-correção: páginas em 31–81 ms completas, `/json` máx 26,6 ms.
3. Estatística de jitter poluída pelo transiente de boot → janela de 60 intervalos.
4. `getDeviceCount()` **cacheado**: a re-varredura de 5 s não re-varria o barramento (nunca detectaria conexão/desconexão em runtime) → `rescan()` passou a chamar `begin()` (re-enumeração real) e revincula/limpa a ROM.
5. Barramento OneWire **intermitente** neste protótipo (mesma ROM alterna presente/ausente; detecção de boot falha e recupera pela re-varredura) → firmware seguro em todas as transições; **P3 (pull-up 4,7 kΩ) segue em aberto**.

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
| HOST — revalidação DEC-07 | 37/37 PASS | `2026-09-12-host-tests-dec07.log` |
| Bateria de proteção — 2ª sessão | 14/14 PASS (janela 1176–1224 ms) | `2026-09-12-bancada-bateria-injecao-dec07.log` |
| Rede (curl 1 Hz, 60 s, sob alarme) | PASS (máx 17 ms; página 12.913 B em 32 ms; `samp=1200`) | `2026-09-12-bancada-rede-dec07.log` |
| Dashboard (navegador real) | PASS (polling 2000 ms; reflexão 1426/1996 ms ≤ 2,5 s) | `2026-09-12-dashboard-dec07.log` + `.png` |
| ROM/barramento (boot e runtime) | PASS* (ROM vinculada `28FFE203B41605C2`; instabilidade física documentada) | `2026-09-12-boot-dec07.log`, `2026-09-12-bancada-sensor-instavel-dec07.log` |
| Boot de produção (final) | PASS (AP + `/json`; `hist_period=1200`) | `2026-09-12-boot-producao-dec07.log` |

## Pendências (ação do operador)

1. **P3 — pull-up OneWire (prioritária)**: confirmar 4,7 kΩ entre DQ (D2) e 3,3 V e a fixação dos fios. Sintoma: a mesma ROM (`28FFE203B41605C2`) alterna presente/ausente em rescans sucessivos e a detecção no boot falha (vínculo só na re-varredura, 5–15 s). Depois, revalidar CA-001 (detecção no boot); CA-003 e CA-014/CA-015 já evidenciados.
2. **CA-013 (audição)** — regravar `bancada`, enviar `TEMP 8000` e confirmar o bipe de ~150 ms a cada ~2 s (lógica já verificada em HOST/injeção).
3. (Opcional) ensaio térmico real 79,9/80,0 °C — QA-4 permite manter injeção como evidência primária.

## Próximo passo

Resolver a pendência 1 (pull-up P3) e revalidar a detecção no boot (CA-001); executar a audição do buzzer (pendência 2); registrar no checklist (`docs/05-testing/controle-termico/2026-09-12-checklist-bancada.md`) e manter a matriz (`spec.md` §9) em dia.

## Cuidados

- Comandos: `-d` pertence ao subcomando (`pio run -d firmware`, `pio test -d firmware -e native`); porta `/dev/ttyUSB0` @ 115200.
- Abrir a serial pode **reiniciar a placa** (motivo registrado como `External System`) e limpar a injeção (volátil) — normal; aguardar o boot antes de enviar comandos.
- Injeção só existe no env `bancada`; para voltar ao estado de entrega: gravar `nodemcuv2`.
- `nmcli` engana com cache de scan — usar `--rescan yes`/`nmcli dev wifi rescan` antes de conectar.
- Não fazer commit automático; alterações de comportamento exigem atualização de `spec.md`/`design.md` antes.
- **Reset por DTR/RTS**: a sequência `setDTR(False); setRTS(True); …` pode deixar a placa em **modo bootloader** (AP desaparece). Recuperação: regravar (`pio run -d firmware -t upload --upload-port /dev/ttyUSB0`) — o upload termina com reset correto.
- Abrir a serial **pode ou não** resetar a placa; o build de produção só emite log no boot — para capturar o banner, ler imediatamente após o upload.
- Quando o AP cai (reset da placa), o NetworkManager volta para `linus`; reconectar com `nmcli con up ESP8266_101026` antes dos testes HTTP.
- **Achado pré-existente (não corrigido nesta tarefa):** `char g_json_buf[cfg::kJsonBufSize]` em `main.cpp` é variável morta (warning `-Wunused-variable` no build do alvo; 1600 B estáticos reservados sem uso — os buffers reais estão em `web_server.cpp` e `console.cpp`). Remover na próxima alteração de `main.cpp` (reduz RAM estática e elimina o warning).

## Critério de conclusão

Itens 1–3 das pendências executados e registrados com evidência; matriz de rastreabilidade sem `PENDENTE` (exceto ensaio térmico opcional, explicitamente dispensado por QA-4).
