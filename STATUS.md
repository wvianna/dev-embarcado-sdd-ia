# STATUS — Sistema de Monitoramento e Controle Térmico (NodeMCU v2 / ESP8266)

| Campo | Valor |
|---|---|
| Data | 2026-09-12 |
| Fase | Reimplementação completa — fechamento (M4) + revisão DEC-07 (M5, 2ª sessão de bancada) |
| Fluxo SDD | GRANDE: `spec.md` → `design.md` → `tasks.md` → implementação → validação de bancada |
| Firmware | `1.0.0` — build de produção (`nodemcuv2`) regravado em `/dev/ttyUSB0` (estado final); env `bancada` validado por injeção |
| Estado geral | HOST 37/37 PASS · bateria 14/14 PASS (janela 1176–1224 ms) · rede/dashboard PASS (polling 2 s) · pendências físicas: pull-up P3 e audição do buzzer |

## Entregue nesta fase

- `firmware/` reimplementado do zero (lógica pura em `lib/thermal_logic` + integração em `src` + testes + ferramenta de bancada).
- Artefatos SDD: `.specs/features/controle-termico/{spec,design,tasks}.md`; `CODEBASE.md` e `ROADMAP.md` atualizados.
- Evidências: `docs/05-testing/controle-termico/` (logs de teste HOST, footprint, bateria de injeção, sessões de rede, boot de produção, screenshots da dashboard, checklist).
- **Revisão DEC-07 (2ª sessão):** ROM do DS18B20 vinculada às conversões/leituras; re-varredura corrigida (`getDeviceCount()` é cacheado — `rescan()` chama `begin()`); amostragem 1200 ms e dashboard com polling de 2000 ms; bateria 14/14, rede e dashboard medidos; produção regravada. Evidências `*-dec07.log` + `2026-09-12-dashboard-dec07.png`.

## Decisões relevantes

- **ADR-011**: `WiFi.setSleepMode(WIFI_NONE_SLEEP)` — elimina atraso de power-save (~3 s) na primeira requisição após ociosidade.
- **ADR-012**: `PIO_FRAMEWORK_ARDUINO_LWIP2_HIGHER_BANDWIDTH` (`TCP_MSS=1460`) — corrige truncamento/engasgo no envio da página (~12,9 KB) e as suspensões de amostragem sob carga HTTP.
- Contrato `/json` ganhou `samp_win_min_ms`/`samp_win_max_ms` (jitter em regime) para evidência de CA-004/CA-021.
- **DEC-07/ADR-013**: amostragem nominal **1200 ms ± 150 ms** (desvio intencional da F1 — decisão do proprietário) e dashboard com **polling de 2000 ms** (reflexão ≤ 2,5 s); conversões/leituras do DS18B20 **vinculadas à ROM** identificada na varredura, com re-varredura de 5 s re-enumerando o barramento (revincula/limpa o vínculo).
- Demais decisões de produto/engenharia: ADR-001…ADR-012 em `design.md` (FSM pura fail-safe, latch volátil com rearme manual, centésimos inteiros no JSON, console só no env `bancada`, etc.).

## Pendências (dependem do operador)

1. **Pull-up OneWire (P3, prioritária)**: confirmar 4,7 kΩ entre DQ (D2) e 3,3 V e a fixação dos fios. Com o sensor físico conectado, a mesma ROM (`28FFE203B41605C2`) alternou presente/ausente em rescans sucessivos e a detecção no boot falhou de forma intermitente (vínculo só na re-varredura, 5–15 s) — o firmware bloqueia e recupera sozinho (evidências `...-sensor-instavel-dec07.log`, `...-boot-dec07.log`).
2. Ouvir o buzzer com alarme injetado (env `bancada`, `TEMP 8000`) → audição de **CA-013** (lógica já verificada em HOST/inyeção).
3. (Opcional) ensaio térmico real na fronteira 79,9/80,0 °C — decisão QA-4 permite injeção como evidência primária.

## Riscos residuais

- **Latch volátil** (sem persistência): reset/watchdog limpa a trava e exige novo rearme — estado seguro por padrão (R7 do CODEBASE).
- **Pull-up do OneWire** a confirmar no hardware físico (P3); enquanto isso, a detecção do sensor é intermitente (bloqueio e recuperação automáticos; CA-001 com limitação registrada).
- Clientes que fecham a conexão keep-alive ociosa (core HTTP) exigem reconexão transparente — absorvida por navegadores; medida em bancada (1 em 60 requisições).

## Próximo passo

Resolver a pendência 1 (pull-up P3) e revalidar a detecção no boot (CA-001); executar a audição do buzzer (pendência 2); manter a matriz (`spec.md` §9) e o checklist de bancada atualizados. Detalhes de continuidade em `HANDOFF.md`.
