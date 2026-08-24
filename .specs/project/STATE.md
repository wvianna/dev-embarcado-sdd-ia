# STATE

## Situação atual

- Constituição do projeto preenchida e aprovada.
- Artefatos de projeto/codebase criados (PROJECT, ROADMAP, TARGET, STACK, ARCHITECTURE, CONVENTIONS, TESTING, CONCERNS).
- Feature `dashboard-esp8266` especificada, projetada, implementada e validada em bancada.

## Decisões registradas

- Plataforma: Arduino Core for ESP8266 >= 3.0.0 via PlatformIO, placa `nodemcuv2`.
- Modo AP, SSID derivado do MAC, rede aberta.
- Dashboard servido pelo MCU em `http://192.168.4.1`.
- Aquisição a 1 Hz, deadline 1,1 s, jitter ±100 ms.
- Sem persistência, sem MQTT, sem modo STA nesta demonstração.
- Scan OneWire no boot identifica o sensor por ROM (`28FFE203B41605C2`).

## Bloqueios

- Tensão/divisor do ADC: A CONFIRMAR.
- Teste visual do dashboard em navegador: pendente (recomendado).

## Próximos passos

1. Teste visual do dashboard (navegador) — CA-004.
2. Medir tensão no A0 e atualizar TARGET/CONCERNS.
3. Testar watchdog/reset em BANCADA.
