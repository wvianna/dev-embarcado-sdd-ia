# Dashboard Web Embarcado — ESP8266 (design)

## Arquitetura

```text
main.cpp
  ├─ setup(): config serial, AP, servidor, sensor, watchdog
  ├─ loop(): aquisição 1 Hz + handleClient() do servidor
  │
  ├─ drivers/
  │   ├─ ds18b20_driver (OneWire + DallasTemperature)
  │   └─ adc_driver (analogRead A0)
  ├─ services/
  │   ├─ sampler (amostra_atual compartilhada)
  │   ├─ json_formatter (payload /api/values)
  │   └─ web_server (ESP8266WebServer, rotas / e /api/values)
    └─ web/
      └─ dashboard_html (PROGMEM: numérico, dois gauges e duas tendências)
```

## Fluxo de dados

```text
loop()
  │  millis() >= próximo tick (1 s)
  ▼
scan OneWire no boot → seleciona ROM da família 0x28 e registra endereço
amostra.temperature = ds18b20.readCelsius(ROM selecionada)   // ou erro
amostra.adc         = analogRead(A0)
amostra.timestamp   = millis()
amostra.sensor_error = (falha do DS18B20)
  │
  ▼
server.handleClient()
  ├─ GET /            → envia dashboard_html (PROGMEM)
  └─ GET /api/values  → json_formatter(amostra)
```

## Estrutura compartilhada

```cpp
struct Sample {
    float temperature;      // °C, NAN em falha
    uint16_t adc;           // 0..1023
    uint32_t timestamp;     // millis()
    bool sensorError;       // falha do DS18B20
};
```

- `Sample` é atualizada apenas no loop de aquisição (sequencial), e lida pelo servidor no mesmo loop — sem concorrência real (single-thread).
- Sem RTOS externo; loop único do Arduino.

## Decisões

- **Servidor:** `ESP8266WebServer` (core) — simples, suficiente para 2 rotas.
- **JSON:** `ArduinoJson` (StaticJsonDocument) — sem alocação dinâmica.
- **Sensor:** `OneWire` + `DallasTemperature`; scan ROM no boot, seleção por família `0x28` e conversão síncrona por leitura (aceitável a 1 Hz).
- **Dashboard:** HTML+JS em PROGMEM com `fetch('/api/values')` a cada 1 s, indicação numérica, gauge e tendência de temperatura, gauge e tendência de A0.
- **Escalas:** temperatura fixa em 20–40 °C; A0 fixa em 0–1023.
- **Histórico:** manter no navegador uma janela limitada de pontos por sinal; descartar o ponto mais antigo ao atingir o limite.
- **Layout:** não exibir quadro separado de “Última atualização”; o timestamp permanece somente no payload JSON.
- **Watchdog:** `ESP.wdtFeed()` no loop; `ESP.wdtDisable()` não utilizado.
- **Clock:** 80 MHz (padrão).

## Alternativas rejeitadas

- **SPIFFS/LittleFS para HTML:** rejeitado para simplificar build/flash; HTML em PROGMEM evita particionar o filesystem.
- **ESPAsyncWebServer:** rejeitado (mais complexo, sem necessidade para 2 rotas a 1 Hz).
- **RTOS (FreeRTOS interno do SDK):** rejeitado nesta versão; loop único atende ao timing de 1 Hz.

## Estados

```text
INIT → AP_SETUP → SERVER_START → RUNNING
                              ↘ FAULT_SENSOR (degradado)
```

- `FAULT_SENSOR`: `sensor_error=true`; dashboard mostra "Erro de sensor"; ADC e HTTP seguem ativos.

## Orçamento de tempo

- Período: 1000 ms; deadline: 1100 ms; jitter: ±100 ms.
- Conversão DS18B20: ~750 ms máx (12 bits) — fica abaixo do período de 1 s; caso exceda, o tick seguinte compensa (deadline 1,1 s).
