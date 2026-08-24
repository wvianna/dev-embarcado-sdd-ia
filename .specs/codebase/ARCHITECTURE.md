# ARCHITECTURE

## Visão geral

```text
Application (dashboard + telemetria)
      │
      ▼
Serviços (acquire, format, serve)
      │
      ▼
Drivers (DS18B20/OneWire, ADC)
      │
      ▼
HAL/SDK (Arduino Core, ESP8266WiFi)
      │
      ▼
MCU ESP8266MOD (NodeMCU V2)
```

## Camadas

- **Application:** estado do sistema, amostra mais recente, lógica do dashboard.
- **Serviços:** aquisição periódica (1 Hz), formatação JSON, servidor HTTP.
- **Drivers:** `Ds18b20` (OneWire + DallasTemperature) e leitura do ADC.
- **HAL/SDK:** Arduino Core, ESP8266WebServer, ESP8266WiFi.

## Fluxo de dados

```text
Timer 1 s (loop com millis)
      ↓
read DS18B20 (OneWire/GPIO4) → temperatura °C (ou falha)
      ↓
read A0 → valor bruto 0..1023
      ↓
amostra_atual { temp, adc, timestamp }
      ↓
GET /            → HTML dashboard (PROGMEM)
GET /api/values  → JSON da amostra
```

## Concorrência e ownership

- Sem RTOS externo: loop principal único (Arduino `loop()`).
- `amostra_atual` é a única estrutura compartilhada entre aquisição e HTTP; acessada de forma sequencial no loop (sem ISR própria para amostragem).
- Servidor HTTP processa requisições no mesmo loop; bloqueios curtos e controlados.

## Estados

```text
INIT → AP_SETUP → SERVER_START → RUNNING
                              ↘ FAULT_SENSOR (degradado, HTTP ativo)
```

- `FAULT_SENSOR`: falha contínua do DS18B20; dashboard indica erro, ADC/HTTP seguem ativos.
