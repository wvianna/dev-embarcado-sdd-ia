# PROJECT

## Identificação

- Nome: Dashboard Web Embarcado — Temperatura e Sinal Analógico
- Objetivo: criar um firmware para ESP8266 (NodeMCU V2 / ESP8266MOD) que expõe um dashboard web com leitura de temperatura (DS18B20 em D2/GPIO4) e sinal analógico (A0, 0 a 1023), operando em modo Access Point.
- Origem: demonstração prática da palestra "SDD para Software Embarcado" (item 42).

## Contexto

O projeto exemplifica o fluxo SDD completo: especificação → design → tarefas → implementação → testes → evidências. É usado como material didático e experimento de laboratório.

## Fora de escopo

- Integração com nuvem / MQTT / broker externo.
- Persistência de dados em flash/EEPROM.
- Baixo consumo / sleep.
- Modo estação (STA) com rede Wi‑Fi existente.
- Atuadores e loops de controle.

## Decisões principais

- Plataforma: Arduino Core for ESP8266 (>= 3.0.0) compilado via PlatformIO.
- Placa: NodeMCU V2 (Amica / ESP-12E / ESP8266MOD).
- Modo AP com SSID derivado do MAC e rede aberta.
- Dashboard servido pelo próprio MCU em `http://192.168.4.1`.
- Validação inicialmente prevista em BANCADA.
