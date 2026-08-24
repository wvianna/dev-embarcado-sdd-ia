# SUMMARY — Dashboard Web Embarcado ESP8266

## Entregáveis

- Projeto PlatformIO em `firmware/` (src, lib, test).
- Especificação, design e tarefas em `.specs/features/dashboard-esp8266/`.
- Artefatos de contexto em `.specs/project/` e `.specs/codebase/`.

## Compilação

- Comando: `pio run -e nodemcuv2` — **SUCCESS**.
- Artefato: `firmware/.pio/build/nodemcuv2/firmware.bin`.
- RAM: 28.928 bytes (35,3% de 81.920) · Flash: 312.031 bytes (29,9%).

## Testes HOST

- Comando: `pio test -e native` — **5/5 PASSED**.
- Casos: sampler válido, falha de sensor, JSON válido, JSON com erro de sensor, faixa ADC.

## Validação em BANCADA

- Upload via `pio run -e nodemcuv2 -t upload --upload-port /dev/ttyUSB0` — SUCCESS.
- Serial 115200: scan OneWire encontrou DS18B20 ROM `28FFE203B41605C2`.
- Temperatura lida: ~29,4–30,1 °C · ADC: 460.
- Wi-Fi: cliente recebeu `192.168.4.100/24` (DHCP).
- `GET /api/values` → `{"temperature":30,"sensor_error":false,"adc":460,"timestamp":...}`.
- `GET /` → HTTP 200, 2162 bytes.

## Critérios de aceite

| Critério | Resultado | Evidência |
| --- | --- | --- |
| CA-001 (SSID derivado do MAC) | PASS | AP `ESP8266-101026` visível |
| CA-002 (DHCP 192.168.4.0/24) | PASS | Cliente recebeu 192.168.4.100/24 |
| CA-003 (scan e identificação DS18B20) | PASS | ROM `28FFE203B41605C2` registrada no boot |
| CA-004 (leitura pelo endereço ROM) | PASS | Temperatura real ~29,4–30,1 °C |
| CA-005 (dashboard em 192.168.4.1) | PASS | GET / → HTTP 200 |
| CA-006 (gauge de temperatura) | PASS | Elemento `gauge` e função `drawGauge` encontrados via HTTP |
| CA-007 (atualização automática) | PASS (código) | JS com fetch periódico de 1 s |
| CA-008 (gráfico de tendência) | PASS | Elemento `trend`, `drawTrend` e histórico de 60 pontos encontrados via HTTP |
| CA-009 (endpoint JSON) | PASS | GET /api/values → JSON válido |
| CA-010 (A0 entre 0 e 1023) | PASS | adc=460 |
| CA-011 (falha do DS18B20) | PASS (código/scan) | modo degradado com HTTP ativo |

## Desvios da especificação

- Gauge e gráfico de tendência implementados no HTML embarcado e validados por smoke test HTTP; teste visual em navegador permanece recomendado.

## Riscos residuais

- Tensão do ADC e divisor do A0 ainda A CONFIRMAR.
- Pull-up do DS18B20 usado internamente; validar robustez com cabo longo.
- Teste de watchdog/reset em BANCADA pendente.
- CA-007 (atualização automática) validado por inspeção do código; recomendado teste visual em navegador.
- CA-006 e CA-008 foram validados por smoke test HTTP; validação visual no navegador permanece recomendada.
