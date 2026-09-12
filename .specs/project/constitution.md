# Constituição do Projeto — Sistema de Monitoramento e Controle Térmico (ESP8266)

> Documento de princípios estáveis e de alta prioridade. Em caso de conflito, prevalece sobre hábitos e preferências locais; regras operacionais ficam em `AGENTS.md`, `.github/instructions/` e nos agentes. O comportamento do produto é normatizado em `docs/descricao.txt`.

## Identidade do alvo

- Produto/sistema: sistema de monitoramento e controle térmico com dashboard web (estudo de caso SDD embarcado).
- MCU e variante: Espressif ESP8266MOD (placa NodeMCU v2, módulo ESP-12E).
- Toolchain/SDK: PlatformIO com Arduino Core para ESP8266; versões fixadas em `firmware/platformio.ini` (`espressif8266@4.2.1`).
- Clock e alimentação: clock padrão do framework, não alterado pelo firmware; alimentação por USB 5 V, I/O em 3,3 V.
- Ambientes de validação disponíveis: `HOST` (testes Unity no env `native`) e `BANCADA` (placa real + injeção). Não há simulador; `HIL` não se aplica a este projeto.

## Princípios obrigatórios

### 1. Segurança e estado seguro

- A segurança térmica tem precedência sobre disponibilidade e conforto de uso.
- Temperatura ≥ 80,0 °C, leitura inválida ou sensor ausente ⇒ resistência desligada (PWM 0) imediatamente.
- O corte por ≥ 80,0 °C é travado (latch) e só sai por rearme manual com leitura válida < 80,0 °C; nenhuma alteração pode contornar esse intertravamento.
- Após boot/reset, a carga inicia desligada até existir leitura válida (sem persistência — decisão registrada).
- Nenhuma alteração pode remover ou afrouxar proteção, limite operacional ou intertravamento sem decisão registrada (`ADR-###`).

### 2. Determinismo e concorrência

- O firmware é estritamente não bloqueante: loop cooperativo com `millis()`.
- Proibidos: `delay()`, interrupções de hardware e timers de hardware.
- Caminhos de tempo declarados: amostragem 1.200 ms ± 150 ms (DEC-07 da feature `controle-termico`); buzzer 150 ms ligado / 2.000 ms desligado na condição de alarme.
- Não há ISR própria nem concorrência preemptiva; buffers são estáticos e de owner único (loop principal).

### 3. Recursos limitados

- Flash (programa): espaço de sketch do alvo = 1.044.464 B; meta de ocupação ≤ 45%.
- RAM: 81.920 B; meta de ocupação estática ≤ 50%; heap livre monitorado via `/json`.
- Sem alocação dinâmica no hot path; sem `String` em acúmulo; HTML do dashboard em PROGMEM.
- Footprint medido e registrado a cada build de alvo.

### 4. Interfaces de hardware e comunicação

- Pinagem congelada: DS18B20 em D2/GPIO4 (OneWire); buzzer ativo em D0/GPIO16 (digital, sem PWM/IRQ); resistência em D1/GPIO5 (PWM 10 bits, `analogWriteRange(1023)`).
- Pull-up do barramento OneWire: **A CONFIRMAR** no hardware físico (típico 4,7 kΩ); não é controlado pelo firmware.
- Rede: AP aberto (decisão de produto registrada), SSID `ESP8266_<3 últimos bytes do MAC>`, 192.168.4.1/24, DHCP ativo, HTTP porta 80.
- Contrato HTTP estável documentado no design da feature: dashboard em `/`, telemetria em `/json`, comandos de carga/rearme em rotas dedicadas.

### 5. Qualidade e rastreabilidade

- IDs `FR-###`, `NFR-###`, `CA-###`, `T-###`, `ADR-###` conforme `.github/instructions/02-sdd-artifacts.instructions.md`.
- Um `PASS` exige evidência registrada; sem hardware, o estado é `PENDENTE`.
- Build de alvo limpo (sem warnings novos) e testes HOST verdes antes de qualquer gravação.
- Código gerado por IA não substitui revisão de contrato, segurança e comportamento observado no alvo.

### 6. Diagnóstico e recuperação

- Serial 115200 com log mínimo; console de bancada e `/json` são a telemetria do sistema.
- Sem persistência (EEPROM/SPIFFS proibidos): evidências são registradas fora do dispositivo.
- Após watchdog/brownout/reset: permanecer em estado seguro (carga desligada; latch volátil) e registrar o motivo quando as APIs do core permitirem.

### 7. Processo de mudança

- Mudança de comportamento exige atualização prévia/acompanhante da especificação (`spec.md`/`design.md`).
- Desvios registrados como `SPEC_DEVIATION` em `tasks.md`/`STATUS.md`.
- Menor alteração suficiente; sem refatoração fora de escopo; sem dependências novas sem justificativa.
- Sem commit automático.

## Gates padrão

- [ ] Requisito e critérios `CA-###` identificados e observáveis.
- [ ] Alvo, toolchain e dependências confirmados.
- [ ] Caminhos de erro/reset considerados (estado seguro).
- [ ] Timing, memória e carga avaliados quando aplicáveis.
- [ ] Evidência no nível correto (`HOST`/`BANCADA`); limitações e pendências registradas.
- [ ] `git diff` executado antes da alteração e escopo mínimo respeitado.
