Aqui está um modelo de `spec.md` voltado para firmware **multicore**, projetado para orientar tanto a equipe de engenharia quanto os assistentes de IA com o rigor necessário.

---

# SPEC: Sistema de Controle e Conectividade Multicore

**Status:** Rascunho / Em Revisão

**Arquitetura Alvo:** Dual-Core (Ex: ESP32, STM32H7, RP2040)

**Versão:** 1.0

---

## 1. Visão Geral do Sistema

O sistema opera em uma arquitetura dual-core onde as funções de **Tempo Real Crítico** (controle de motores e leitura de sensores) e **Conectividade / I/O Lento** (Wi-Fi, Bluetooth e gerenciamento de arquivos) são isoladas em núcleos distintos.

---

## 2. Alocação de Núcleos (Core Affinity)

| Núcleo | Papel / Responsabilidade | Prioridade das Tasks | Restrições |
| --- | --- | --- | --- |
| **Core 0** | Protocolos de Rede, Pilha Bluetooth, Interface CLI e Persistência na Flash. | Média / Baixa (Cooperativo) | Proibido executar loops de espera ativa (*busy-wait*) > 10µs. |
| **Core 1** | Malha de Controle de Tempo Real (10 kHz), Leitura de ADC e Interrupções do Motor. | Alta (Preemptivo / Hard Real-Time) | Não pode realizar chamadas bloqueantes de I/O, rede ou gravação na Flash. |

---

## 3. Gestão de Memória e Coerência (Shared Memory)

* **RAM Compartilhada (`SRAM_SHARED`):**
* **Endereço / Seção:** `.shared_ram` (Endereço base: `0x24000000`, Tamanho: 8 KB).
* **Atributos:** Declarada como `volatile` e sem cache (`non-cacheable`) para evitar inconsistências de cache entre os núcleos.


* **Tightly-Coupled Memory (TCM):**
* **Core 1 Code/Data:** A ISR do motor e o buffer do ADC devem residir na `ITCM`/`DTCM` do Core 1 para execução com latência zero de barramento.



---

## 4. Ownership de Hardware (Periféricos)

* **Core 0:** Proprietário exclusivo de `UART1`, `SPI2` (Flash Externa) e `Wi-Fi Hardware`.
* **Core 1:** Proprietário exclusivo de `TIM1` (PWM), `ADC1` e `GPIO_PORT_A`.
* **Regra de Acesso:** Nenhum núcleo pode escrever nos registradores de hardware pertencentes ao outro núcleo. A comunicação deve ocorrer via IPC.

---

## 5. Comunicação Inter-Núcleos (IPC) e Sincronização

```
[ Core 0 (Network) ] --(Hardware Ring Buffer / Shared RAM)--> [ Core 1 (Control) ]
           |                                                        |
           +--------------------(Hardware Spinlock)-----------------+

```

### 5.1 Primitivas de Sincronização

* **Hardware Spinlock (ID: `SPINLOCK_SHARED_TELEMETRY`):** Protege a escrita e leitura do buffer circular de telemetria na `SRAM_SHARED`.
* **Tempo Máximo de Retenção (Hold Time):** $\le 2\ \mu\text{s}$.
* **Comportamento em Falha:** Se o lock não for adquirido em $5\ \mu\text{s}$, a chamada descarta o pacote atual e incrementa o contador de erros `telemetry_drop_cnt`.



### 5.2 Protocolo de Mensagens (Mailbox / IPC)

* **Transporte:** Fila *lock-free* do tipo Produtor-Consumidor baseada na memória compartilhada + Interrupção de Software por Hardware Mailbox (SGI).
* **Fluxo de Evento (Core 0 $\rightarrow$ Core 1):**
1. Core 0 escreve o novo comando (`cmd_setpoint_t`) no buffer ring.
2. Core 0 dispara a interrupção `IPC_SGI_0`.
3. Core 1 atende a ISR de `IPC_SGI_0`, consome o dado do buffer e atualiza as variáveis internas de controle.



---

## 6. Análise de Impacto no Timing e Restrições Temporais

* **Latência Máxima de Interrupção (Core 1):** $\le 1.5\ \mu\text{s}$.
* **Contenção de Barramento (Flash Operations):** Quando o Core 0 executa gravação/apagamento na Flash, o barramento principal pode sofrer contenção de até $2\text{ ms}$.
* *Mitigação:* O código do Core 1 deve rodar $100\%$ a partir da RAM/ITCM durante a gravação da Flash para evitar paralisação do algoritmo de controle (*stall*).


* **Deadline do Loop de Controle (Core 1):** $100\ \mu\text{s}$ (frequência de 10 kHz). O tempo máximo de execução da rotina não pode exceder $35\ \mu\text{s}$ por ciclo.