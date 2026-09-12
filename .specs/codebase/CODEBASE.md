# CODEBASE — Fatos consolidados do repositório

> Documento consolidado de `codebase/` (STACK, TARGET, ARCHITECTURE, CONVENTIONS, TESTING, CONCERNS), conforme decisão de consolidação registrada em `STATUS.md` (projeto de firmware único).
> Fontes: `docs/descricao.txt` (normativa do produto), `AGENTS.md` (regras permanentes), `.specs/project/constitution.md` (princípios) e o código em `firmware/`.

## STACK

| Item | Valor |
|---|---|
| Build system | PlatformIO Core 6.1.19 |
| Plataforma alvo | `espressif8266@4.2.1` (Arduino Core para ESP8266) |
| Ambientes | `nodemcuv2` (default), `native` (testes HOST/Unity), `bancada` (`-DBENCH_TEMP_INJECTION=1`) |
| Bibliotecas | `OneWire@2.3.8`, `DallasTemperature@3.11.0` |
| Serial / upload | 115200 baud; upload em `/dev/ttyUSB0` |
| Ferramenta de bancada | Python 3 + `pyserial` (`firmware/tools/bench_injection_test.py`) |

## TARGET

- MCU: ESP8266MOD (NodeMCU v2), I/O 3,3 V, flash 4 MB (32 Mbit).
- Espaço de sketch do env `nodemcuv2`: 1.044.464 B; RAM: 81.920 B.
- Pinagem fixa (não violar):
  - DS18B20: **D2/GPIO4**, barramento OneWire;
  - buzzer ativo: **D0/GPIO16**, somente `digitalWrite` (sem PWM e sem interrupção);
  - resistência de aquecimento: **D1/GPIO5**, PWM por software 0–1023 (`analogWriteRange(1023)`).
- Sem ISR própria, sem timers de hardware, sem persistência (EEPROM/SPIFFS proibidos).

## ARCHITECTURE (visão)

- `firmware/lib/thermal_logic/` — lógica pura (sem `Arduino.h`), testável em HOST: `PeriodicTimer`, `BuzzerPattern`, `ControlPolicy`, `TrendBuffer`, `StatusJson`.
- `firmware/src/` — integração com o SDK do ESP8266: `config.h`, `main.cpp`, `ds18b20_sensor`, `web_server`, `web/dashboard_html.h`, `console` (bancada).
- Fluxo: varredura OneWire no boot (ROM vinculada) → amostragem (1,2 s) → `ControlPolicy` (segurança/latch) → atuação (PWM/buzzer) → telemetria (`/json`, console serial).
- Contrato HTTP: `/` (dashboard), `/json` (telemetria), rotas de comando da carga e rearme.
- Detalhes e ADRs: `.specs/features/controle-termico/design.md`.

## CONVENTIONS

- C++17 (`gnu++17`); identificadores de código em inglês, comentários e documentação em pt-BR.
- Temperatura em centésimos de °C (`int16_t`); sem `float` no JSON; formatação com `snprintf` em buffer estático.
- Sem `std::string`/`std::vector` no alvo; sem alocação dinâmica no loop; sem `String` em acúmulo.
- Constantes de pinos, tempos e limiares centralizadas em `firmware/src/config.h`.
- HTML/CSS/JS do dashboard em PROGMEM, sem CDN externo (AP sem internet).
- Comportamento novo: primeiro na `lib` pura com teste HOST; depois integração em `src`.

## TESTING

- Gate mínimo: `pio test -d firmware -e native` verde + `pio run -d firmware` limpo (footprint medido).
- Níveis de evidência: `HOST`, `SIMULADOR` (não aplicável), `BANCADA`, `HIL` (não aplicável).
- Bancada: env `bancada` com injeção de temperatura e falha via serial + script Python; medição de jitter de amostragem e de latência HTTP.
- Proibido declarar validação de hardware sem execução física; pendências explícitas em `STATUS.md`/`HANDOFF.md`.

## CONCERNS

- **AP aberto**: decisão de produto registrada na especificação; risco aceito para rede local sem dados sensíveis — revisar antes de qualquer uso além do estudo de caso.
- **Latch volátil** (sem persistência): reset/watchdog limpa o latch e desliga a carga — estado seguro, mas exige novo rearme após eventos.
- **Falha de sensor não é latcheada**: a carga permanece bloqueada enquanto a leitura for inválida e retoma quando a leitura voltar a ser válida (decisão registrada; risco residual documentado).
- **Jitter de amostragem sob carga HTTP**: medido em bancada; limite de ±150 ms.
- **Validações dependentes de operador**: audição do buzzer e desconexão física do sensor (D2) são pendências de bancada quando não executadas.
- **Efeitos elétricos**: pull-up do OneWire a confirmar no hardware; ruído com cabeamento longo é risco físico fora do escopo do firmware.
