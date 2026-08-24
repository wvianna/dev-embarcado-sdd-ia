# CONVENTIONS

## Código

- C++17 compatível com Arduino Core for ESP8266.
- Nomes: `camelCase` para variáveis, `PascalCase` para classes/funções de API, `SCREAMING_SNAKE_CASE` para constantes.
- Sem `malloc` no caminho periódico; evitar alocação dinâmica no atendimento HTTP.
- Strings do dashboard em `PROGMEM` para economizar RAM.
- Funções curtas e com responsabilidade única.

## IDs e rastreabilidade

- Requisitos funcionais: `FR-###`.
- Requisitos não funcionais: `NFR-###`.
- Tarefas: `T-###`.
- Critérios de aceitação: `CA-###`.

## Build

- Compilar via `pio run` (env `nodemcuv2`).
- Tratar warnings como pendência até decisão da política; não silenciar por padrão.

## GPIO e pinagem

- DS18B20: GPIO4 (D2).
- ADC: A0.
- Não alterar pinagem sem atualizar `TARGET.md` e `constitution.md`.

## Serial

- Baud rate 115200.
- Logs curtos, prefixados por nível (INFO, WARN, ERROR).
