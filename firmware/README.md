# Dashboard Web Embarcado — ESP8266 (firmware)

Firmware para NodeMCU V2 (ESP8266MOD) que adquire temperatura (DS18B20 em D2/GPIO4) e sinal analógico (A0, 0–1023) a 1 Hz e apresenta um dashboard web servido pelo próprio MCU em modo Access Point. O dashboard possui indicação numérica, gauge e gráfico de tendência da temperatura (20–40 °C) e do ADC (0–1023), com histórico limitado a 60 pontos e sem quadro separado de última atualização.

## Requisitos de hardware

- NodeMCU V2 / ESP8266MOD
- DS18B20 em D2 (GPIO4) via OneWire
- Sinal analógico em A0 (0–3,3 V → 0–1023)

## Configuração

- Arduino Core for ESP8266 >= 3.0.0
- PlatformIO
- Baud rate serial: 115200

## Compilar

```bash
pio run -e nodemcuv2
```

## Gravar

```bash
pio run -e nodemcuv2 -t upload --upload-port /dev/ttyUSB0
```

## Testes HOST

```bash
pio test -e native
```

## Usar

1. Energize o NodeMCU; o AP aparece com SSID derivado do MAC (ex.: `ESP8266-101026`), rede aberta.
2. Conecte-se ao AP; o DHCP entrega IP na sub-rede `192.168.4.0/24`.
3. Abra `http://192.168.4.1` no navegador.
4. Endpoint JSON: `http://192.168.4.1/api/values`.

## Monitorar serial

```bash
pio device monitor -p /dev/ttyUSB0 -b 115200
```

No boot, o firmware executa um scan OneWire e imprime os endereços ROM encontrados (ex.: `28FFE203B41605C2`).

## Estrutura

```text
src/
  main.cpp                # boot, AP, loop de aquisição
  ds18b20_driver.*        # scan OneWire + leitura DS18B20
  web_server.*            # servidor HTTP (rotas / e /api/values)
  web/dashboard_html.h    # dashboard em PROGMEM
lib/logic/                # sampler, JSON, ADC (testáveis em host)
test/                     # testes HOST (Unity)
```

Especificação e design: `.specs/features/dashboard-esp8266/`.
