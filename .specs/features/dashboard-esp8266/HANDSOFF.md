# HANDSOFF — Dashboard Web Embarcado ESP8266

## Estado atual

Firmware funcional e gravado no NodeMCU (ESP8266MOD). AP, servidor HTTP, DS18B20, ADC, gauge e gráfico de tendência implementados e validados por smoke test HTTP; validação visual permanece recomendada.

## Objetivo restante

- Validar visualmente gauge e gráfico de tendência no navegador (CA-006 e CA-008).
- Validar comportamento visual do dashboard em navegador (atualização automática, CA-007).
- Confirmar tensão/divisor do A0 (hardware).
- Testar watchdog e reset em BANCADA.

## Arquivos relevantes

- `firmware/src/main.cpp` — boot, AP, loop de aquisição.
- `firmware/src/ds18b20_driver.*` — scan OneWire e leitura do DS18B20.
- `firmware/lib/logic/*` — sampler, JSON, ADC (testáveis em host).
- `firmware/src/web_server.*` e `firmware/src/web/dashboard_html.h`.
- `.specs/features/dashboard-esp8266/*` — spec, design, tasks, summary.

## Decisões tomadas

- Arduino Core for ESP8266 >= 3.0.0 via PlatformIO, placa `nodemcuv2`.
- Dashboard em PROGMEM (sem SPIFFS).
- Servidor `ESP8266WebServer` (2 rotas).
- Scan OneWire no boot para identificar o sensor por ROM.

## Comandos executados

- `pio run -e nodemcuv2` — SUCCESS.
- `pio test -e native` — 5/5 PASSED.
- `pio run -e nodemcuv2 -t upload --upload-port /dev/ttyUSB0` — SUCCESS.
- `pio device monitor -p /dev/ttyUSB0 -b 115200` — boot e leituras OK.

## Bloqueios de hardware/ambiente

- Nenhum bloqueio ativo. Hardware disponível em `/dev/ttyUSB0`.

## Próximos passos

1. Conectar navegador ao AP `ESP8266-101026` e abrir `http://192.168.4.1`.
2. Verificar visualmente gauge e gráfico de tendência (CA-006 e CA-008).
3. Verificar atualização automática do dashboard (CA-007).
4. Medir tensão no A0 e ajustar TARGET/CONCERNS.
5. Testar reset/watchdog.

## Critério para considerar concluído

Todos os critérios de aceite com evidência registrada, incluindo teste visual do dashboard e confirmação da tensão do ADC.
