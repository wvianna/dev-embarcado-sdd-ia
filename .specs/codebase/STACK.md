# STACK

## Linguagens e plataformas

- C++ (Arduino framework para ESP8266)
- HTML + JavaScript (dashboard embarcado, PROGMEM)
- JSON (endpoint de telemetria)

## Ferramentas

- PlatformIO (build, upload, test)
- Arduino Core for ESP8266 >= 3.0.0
- Serial 115200 para debug

## Bibliotecas previstas

- `OneWire` (Dallas/Miles Burton)
- `DallasTemperature` (DS18B20)
- `ESP8266WebServer` (core) — servidor HTTP
- `ESP8266WiFi` (core) — modo AP
- `ArduinoJson` — geração do JSON do endpoint

## Testes

- HOST: testes unitários em C++/Unity via PlatformIO `test/`
- BANCADA: avaliação em hardware real (serial + navegador)

## Dependências pendentes

- Capacidade exata de flash/RAM: A CONFIRMAR
- Tensão do ADC e divisor de entrada: A CONFIRMAR
