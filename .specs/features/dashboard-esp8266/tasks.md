# Dashboard Web Embarcado — ESP8266 (tasks)

## T-001 Criar esqueleto do projeto PlatformIO

- Requisitos: — (infraestrutura)
- Onde: `firmware/platformio.ini`, `firmware/src/main.cpp` (esqueleto)
- Depende de: nenhum
- Reutiliza: —
- Feito quando: `pio run` compila um hello minimalista sem erros
- Testes: build
- Gate: `pio run -e nodemcuv2` com sucesso

## T-002 Implementar driver DS18B20 e scan OneWire

- Requisitos: FR-001, FR-002, FR-003, FR-011
- Onde: `firmware/src/ds18b20_driver.h/.cpp`
- Depende de: T-001
- Reutiliza: OneWire + DallasTemperature
- Feito quando: enumera ROMs no boot, identifica família `0x28`, registra o endereço selecionado, retorna temperatura em °C e sinaliza falha
- Testes: build + smoke serial (ROM listada)
- Gate: scan ROM confirmado no log serial; build ok

## T-003 Implementar dashboard numérico, gauge e tendência

- Requisitos: FR-004, FR-005, FR-006, FR-007, FR-008, FR-012
- Onde: `firmware/src/web/dashboard_html.h`
- Depende de: T-002
- Reutiliza: endpoint JSON
- Feito quando: dashboard mostra números, gauge e tendência de temperatura (20–40 °C), gauge e tendência de A0 (0–1023), sem quadro de última atualização
- Testes: inspeção no navegador + smoke HTTP
- Gate: três visualizações presentes e sem crescimento indefinido do histórico

## T-004 Implementar driver ADC

- Requisitos: FR-009, NFR-005
- Onde: `firmware/src/adc_driver.h/.cpp`
- Depende de: T-001
- Reutiliza: `analogRead()`
- Feito quando: retorna uint16_t 0..1023
- Testes: HOST (validação de faixa)
- Gate: testes HOST passam; build ok

## T-005 Implementar sampler (amostra a 1 Hz)

- Requisitos: FR-010, FR-011, NFR-001
- Onde: `firmware/src/sampler.h/.cpp`
- Depende de: T-002, T-004
- Reutiliza: drivers
- Feito quando: atualiza `Sample` a cada 1 s com timestamp
- Testes: HOST (periodicidade via clock injetado)
- Gate: testes HOST passam; build ok

## T-006 Implementar formatação JSON

- Requisitos: FR-013
- Onde: `firmware/lib/logic/json_formatter.h/.cpp`
- Depende de: T-005
- Reutiliza: ArduinoJson
- Feito quando: gera JSON válido com temperature/adc/timestamp/sensor_error
- Testes: HOST (parse do payload)
- Gate: testes HOST passam; build ok

## T-007 Implementar servidor HTTP e dashboard

- Requisitos: FR-012, FR-013, FR-014, FR-015
- Onde: `firmware/src/web_server.h/.cpp`, `firmware/src/web/dashboard_html.h`
- Depende de: T-006
- Reutiliza: ESP8266WebServer, ESP8266WiFi
- Feito quando: AP ativo com SSID derivado do MAC, rotas / e /api/values respondem
- Testes: build + smoke (serial)
- Gate: build ok; rotas declaradas

## T-008 Integrar no main e watchdog

- Requisitos: NFR-003, NFR-004
- Onde: `firmware/src/main.cpp`
- Depende de: T-007
- Reutiliza: todos os módulos
- Feito quando: boot → AP → servidor → loop 1 Hz com wdtFeed
- Testes: build + smoke (serial)
- Gate: build ok; log de boot limpo

## T-009 Validar em BANCADA

- Requisitos: CA-001 a CA-013
- Onde: hardware físico
- Depende de: T-008
- Reutiliza: —
- Feito quando: AP visível, dashboard acessível, valores reais exibidos
- Testes: BANCADA (serial + navegador)
- Gate: critérios de aceite registrados (PASS/FAIL/PENDENTE)

## Entregáveis e aceite

- Arquivos esperados: `firmware/src/*`, `firmware/test/*`, `platformio.ini`, README.
- Build alvo: `pio run -e nodemcuv2`.
- Testes: `pio test -e native` (HOST).
- Aceite rastreado: CA-001..CA-013.
- Pendências: validação física de BANCADA (hardware), tensão do ADC, pull-up OneWire.
