# STATUS — Sistema de Monitoramento e Controle Térmico (NodeMCU v2 / ESP8266)

| Campo | Valor |
|---|---|
| Data | 2026-09-12 |
| Fase | Reimplementação completa — fechamento (M4) |
| Fluxo SDD | GRANDE: `spec.md` → `design.md` → `tasks.md` → implementação → validação de bancada |
| Firmware | `1.0.0` — build de produção (`nodemcuv2`) gravado em `/dev/ttyUSB0`; env `bancada` validado por injeção |
| Estado geral | HOST 37/37 PASS · bateria de bancada 14/14 PASS · rede/HTTP PASS · pendências físicas assistidas (operador) |

## Entregue nesta fase

- `firmware/` reimplementado do zero (lógica pura em `lib/thermal_logic` + integração em `src` + testes + ferramenta de bancada).
- Artefatos SDD: `.specs/features/controle-termico/{spec,design,tasks}.md`; `CODEBASE.md` e `ROADMAP.md` atualizados.
- Evidências: `docs/05-testing/controle-termico/` (logs de teste HOST, footprint, bateria de injeção, sessões de rede, boot de produção, screenshots da dashboard, checklist).

## Decisões relevantes

- **ADR-011**: `WiFi.setSleepMode(WIFI_NONE_SLEEP)` — elimina atraso de power-save (~3 s) na primeira requisição após ociosidade.
- **ADR-012**: `PIO_FRAMEWORK_ARDUINO_LWIP2_HIGHER_BANDWIDTH` (`TCP_MSS=1460`) — corrige truncamento/engasgo no envio da página (~12,9 KB) e as suspensões de amostragem sob carga HTTP.
- Contrato `/json` ganhou `samp_win_min_ms`/`samp_win_max_ms` (jitter em regime) para evidência de CA-004/CA-021.
- Demais decisões de produto/engenharia: ADR-001…ADR-010 em `design.md` (FSM pura fail-safe, latch volátil com rearme manual, centésimos inteiros no JSON, console só no env `bancada`, etc.).

## Pendências (dependem do operador)

1. Conectar o DS18B20 no D2/GPIO4 → validar **CA-001/CA-003** (detecção/ROM, leituras válidas, re-varredura de 5 s, retomada em runtime).
2. Desconectar/reconectar o sensor em operação → variante física de **CA-015** (corte + alerta + retomada sem religamento).
3. Ouvir o buzzer com alarme injetado (env `bancada`, `TEMP 8000`) → audição de **CA-013** (lógica já verificada em HOST/injeção).
4. (Opcional) ensaio térmico real na fronteira 79,9/80,0 °C — decisão QA-4 permite injeção como evidência primária.

## Riscos residuais

- **Latch volátil** (sem persistência): reset/watchdog limpa a trava e exige novo rearme — estado seguro por padrão (R7 do CODEBASE).
- **Pull-up do OneWire** a confirmar no hardware físico (P3).
- Clientes que fecham a conexão keep-alive ociosa (core HTTP) exigem reconexão transparente — absorvida por navegadores; medida em bancada (1 em 60 requisições).

## Próximo passo

Executar o roteiro de pendências físicas (seção "Pendências") e atualizar a matriz de rastreabilidade (`spec.md` §9) para `PASS` nos itens concluídos. Detalhes de continuidade em `HANDOFF.md`.
