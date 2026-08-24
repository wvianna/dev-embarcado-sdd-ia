# TESTING

## Níveis previstos

| Nível | O que cobre | Como executar |
|---|---|---|
| HOST | Lógica de formatação JSON, validação de faixa A0, mensagens de erro | `pio test -e native` |
| BANCADA | AP visível, IP 192.168.4.1, dashboard, leitura real DS18B20/A0 | Serial + navegador |

## Resultados registrados

- HOST: 5/5 PASS (`pio test -e native`).
- BANCADA: AP `ESP8266-101026`, DHCP 192.168.4.100/24, HTTP 200 em /, JSON válido em /api/values, DS18B20 ROM `28FFE203B41605C2`, temp ~29–30 °C, ADC 460.

## Casos HOST

- `test_format_json`: gera JSON com temperatura/ADC/timestamp válidos.
- `test_adc_range`: valores fora de 0..1023 são tratados/rejeitados.
- `test_sensor_error`: falha do sensor produz marcador de erro sem quebrar o JSON.

## Registro de evidência

Cada critério de aceite registra `PASS`, `FAIL` ou `PENDENTE` no `SUMMARY.md`.

## Pendências

- Validação elétrica do ADC (tensão real) — BANCADA.
- Leitura real do DS18B20 (conexão física) — BANCADA.
- Teste de watchdog em reset — BANCADA.
