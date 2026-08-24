# ROADMAP

## Fase 1 — Esqueleto e contexto (concluída)

- [x] Constituição preenchida (`.specs/project/constitution.md`).
- [x] TARGET com identificação de hardware/toolchain.
- [x] Estrutura `.specs/` criada.

## Fase 2 — Especificação da feature (concluída)

- [x] `spec.md` com requisitos FR/NFR e critérios de aceitação.
- [x] `design.md` com arquitetura e fluxo de dados.
- [x] `tasks.md` com quebra atômica e gates.

## Fase 3 — Implementação (concluída)

- [x] Projeto PlatformIO em `firmware/`.
- [x] Firmware: AP, servidor HTTP, DS18B20 (com scan OneWire), A0, watchdog.
- [x] Dashboard HTML/JS embarcado com indicação numérica, gauge e gráfico de tendência.
- [x] Endpoint JSON `/api/values`.

## Fase 4 — Verificação (parcialmente concluída)

- [x] Testes HOST (lógica, JSON, faixas) — 5/5 PASS.
- [x] Compilação para o alvo (PlatformIO build) — SUCCESS.
- [x] Validação em BANCADA — AP, DHCP, HTTP e leituras OK.
- [ ] Inspeção visual final do gauge e gráfico de tendência no navegador.
- [ ] Medição da tensão do ADC (pendente).

## Fase 5 — Encerramento (em andamento)

- [x] `README.md` com build/flash/uso.
- [x] `SUMMARY.md` e `HANDSOFF.md`.
