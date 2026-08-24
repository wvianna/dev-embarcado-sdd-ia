# TARGET

## Hardware

- MCU/variante: módulo Espressif ESP8266MOD
- Placa: NodeMCU V2 (Amica / ESP-12E)
- Clock: 80 MHz padrão (configurável para 160 MHz)
- Alimentação: 5 V via Micro-USB; regulador interno AMS1117 para 3,3 V
- Flash: 4 MB (ESP-12E) — A CONFIRMAR capacidade efetiva
- RAM: ~80 KB disponível para usuário (aplicação) — A CONFIRMAR
- GPIO: D2/GPIO4 (DS18B20 OneWire)
- ADC: A0 (0 a 3,3 V → 0 a 1023)

## Software/Toolchain

- Framework: Arduino Core for ESP8266
- Versão mínima: 3.0.0
- Build: PlatformIO (`platformio.ini`)
- Placa alvo: `nodemcuv2`
- Baud rate (serial): 115200

## Interfaces

- OneWire: GPIO4 (D2)
- ADC: A0
- Wi-Fi: modo AP, rede aberta, SSID derivado do MAC
- HTTP: porta 80 em `192.168.4.1`

## Rede

- Sub-rede: `192.168.4.0/24`
- IP do MCU/AP: `192.168.4.1` (fixo)
- Gateway: `192.168.4.1`
- Máscara: `255.255.255.0`
- DHCP: ativo no AP, entregando IP aos clientes
