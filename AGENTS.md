# AGENTS.md — Regras Permanentes do Projeto

> **Nenhum agente pode alterar código sem antes ler este arquivo.**
> Sistema de Monitoramento e Controle Térmico ESP8266 (NodeMCU v2) — estudo de caso SDD embarcado.
> Documentação em pt-BR; identificadores de código em inglês; comentários de código em pt-BR.

## 1. Ordem de leitura obrigatória

1. `AGENTS.md` (este arquivo)
2. `.specs/project/constitution.md`
3. `STATUS.md` e `HANDOFF.md` (trabalho contínuo)
4. `.specs/features/<recurso>/spec.md` → `design.md` → `tasks.md` (conforme a tarefa)
5. Código existente do módulo afetado

Em caso de conflito: `spec.md`/`design.md` aprovados prevalecem sobre hábitos; a constituição prevalece sobre preferências locais; `docs/descricao.txt` é a fonte original do produto (não editar).

## 2. Visão geral

Firmware para NodeMCU v2 (ESP8266): monitora temperatura via DS18B20 (OneWire), controla resistência de aquecimento por PWM (0–1023), alarme sonoro em ≥80 °C, AP Wi-Fi aberto com dashboard web e endpoint JSON. Toda a lógica é não bloqueante no loop principal.

## 3. Estrutura de diretórios

```text
firmware/                  # Projeto PlatformIO (código do firmware)
  platformio.ini           # envs: nodemcuv2 (default), native (testes host), bancada (injeção)
  src/                     # Código específico de hardware/ESP8266
  lib/thermal_logic/       # Lógica pura portável (sem Arduino.h) — testável em HOST
  test/                    # Testes Unity (executados no env native)
.specs/                    # Especificação técnica SDD (project/, codebase/, features/)
docs/                      # Planejamento, relatórios, segurança, release, rastreabilidade
.github/                   # Skills e definições de agentes (não alterar sem motivo)
```

## 4. Comandos importantes

| Ação | Comando |
|---|---|
| Compilar para o alvo | `pio run -d firmware` |
| Testes host (lógica pura) | `pio test -d firmware -e native` |
| Gravar no ESP | `pio run -d firmware -t upload --upload-port /dev/ttyUSB0` |
| Monitor serial | `pio device monitor -d firmware -p /dev/ttyUSB0 -b 115200` |
| Gravar build de bancada | `pio run -d firmware -e bancada -t upload --upload-port /dev/ttyUSB0` |

Porta serial: `/dev/ttyUSB0` (115200 baud). Conectar ao AP do ESP para testes HTTP (SSID `ESP8266_XXXXXX`, IP 192.168.4.1).

## 5. Restrições de hardware (não violar)

- Pinagem fixa: DS18B20 → **D2/GPIO4** (OneWire); buzzer ativo → **D0/GPIO16**; resistência → **D1/GPIO5** (PWM).
- **GPIO16 não suporta PWM nem interrupções** — usar apenas `digitalRead/digitalWrite`.
- PWM da resistência: `analogWriteRange(1023)` + `analogWrite` (10 bits). 0 = desligado; 1023 = potência máxima.
- **Proibido**: interrupções de hardware, timers para amostragem, `delay()` bloqueante, EEPROM/SPIFFS/persistência.
- Amostragem: 1200 ms ±150 ms (DEC-07), não bloqueante com `millis()`; varredura OneWire no boot vincula a ROM do DS18B20 e a conversão/leitura usam esse endereço (re-varredura de 5 s revincula); conversão assíncrona (`setWaitForConversion(false)`, ler ao final do ciclo e re-disparar).
- OneWire (ADR-014): firmware habilita o **pull-up interno** do GPIO4 e faz 3 tentativas de varredura no boot / 2 por re-varredura, com 1 retry imediato na leitura; o resistor externo de **4,7 kΩ** continua recomendado (P3).
- Threshold de segurança: `>= 80.0 °C` → PWM 0 imediato + latch até rearme manual; buzzer 150 ms ON / 2000 ms OFF enquanto a condição persistir.
- Sem sensor detectado no boot → carga bloqueada; re-scan OneWire a cada 5 s; sem leitura válida → carga bloqueada.
- AP aberto (sem senha) — decisão de produto registrada; IP fixo 192.168.4.1/24; DHCP ativo; HTTP porta 80.

## 6. Convenções de código

- C++17 (`gnu++17`); sem exceções/STL pesada no alvo (evitar `std::string`, `std::vector` no hot path).
- Toda lógica pura (FSM, políticas, agendamento, JSON) em `firmware/lib/thermal_logic/` **sem** incluir `Arduino.h` — é o que garante o teste em HOST.
- Código dependente do SDK (`WiFi`, `ESP8266WebServer`, `OneWire`, `DallasTemperature`, GPIO) em `firmware/src/`.
- Constantes de pinos, tempos e limiares centralizadas em `firmware/src/config.h` (e duplicadas apenas quando estritamente necessário na lógica pura, com teste).
- Strings de UI (dashboard) em pt-BR; HTML embutido em flash (PROGMEM), sem CDN externo (o AP não tem internet).
- Proibido `String` em acúmulo no loop; buffers estáticos com tamanho definido.
- Sem variáveis globais mutáveis acessadas fora do contexto do loop (não há ISR própria).

## 7. Testes e evidência

- Lógica pura: testes Unity obrigatórios em `firmware/test/` (env `native`) — gate mínimo antes de qualquer entrega.
- Alvo: `pio run -d firmware` deve compilar limpo; medir sketch/RAM e registrar.
- Bancada: flash real + monitor serial + AP/HTTP; registrar evidências em `docs/05-testing/`.
- **Nunca declarar hardware validado com base apenas em teste HOST.** Níveis de evidência: `HOST`, `SIMULADOR`, `BANCADA`, `HIL` (simulador não se aplica aqui).

## 8. Processo de mudança

- Mudança de comportamento exige atualização prévia/acompanhante de `spec.md` (IDs `FR-###`/`NFR-###`) e `design.md` quando aplicável.
- Atualizar `STATUS.md` ao final de cada fase; `HANDOFF.md` quando houver pendência de validação (ex.: teste físico/HIL).
- Registrar desvios da especificação como `SPEC_DEVIATION` (em `tasks.md`/`STATUS.md`).
- Não fazer commit automaticamente; commits atômicos apenas quando solicitado.
- Classificação de fluxo (skill SDD): escopo pequeno → `.specs/quick/`; elevação obrigatória para fluxo completo se tocar ISR/DMA/registradores/seção crítica/energia/Flash.

## 9. Licença

Apache License 2.0 — Copyright 2026 William. Detalhes no `README.md`.
