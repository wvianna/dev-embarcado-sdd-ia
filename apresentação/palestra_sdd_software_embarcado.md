---
marp: true
theme: default
paginate: true
---

# SDD para Software Embarcado
## Da especificação ao firmware em microcontroladores

**Palestra:** desenvolvimento orientado a especificações com apoio de LLMs  
**Ambiente:** VS Code + arquivos `.md` + Git  
**Foco:** firmware, drivers, RTOS, testes e validação

> **Mensagem central:** SDD não elimina engenharia. Ele torna a intenção, as decisões, as restrições e as evidências explícitas e rastreáveis.

---

# 1. O que vamos aprender

- O que são **LLM, tokens, janela de contexto e custo**
- Execução de LLM **local × provedor remoto**
- O que é **harness** e por que ele importa
- O conceito de **SDD — Specification-Driven Development**
- Como aplicar SDD ao **software embarcado**
- Fundamentos de MCU necessários para conversar corretamente com uma IA
- Como usar `SKILL.md` e `constitution.md` dentro do VS Code
- Como transformar uma funcionalidade em:
  - especificação
  - design
  - tarefas
  - implementação
  - testes
  - evidências
- Como distinguir **HOST, SIMULADOR, BANCADA e HIL**

---

# 2. Uma provocação

## "Se a IA escreve o código, o programador acabou?"

**Não.**

Em software embarcado, a pergunta correta é:

> **Quem definiu o comportamento que o código deve implementar?**

E ainda:

- Quais são os limites?
- Qual é o timing?
- O que acontece quando o sensor falha?
- O que acontece no reset?
- Quanto de RAM pode ser usado?
- Quanto de flash pode ser consumido?
- Qual é o estado seguro?
- Como provar que o código funciona no alvo?

**SDD muda o foco de "pedir código" para "definir e verificar comportamento".**

**Ler datasheet é essencial (ou pedir para uma AI auxiliar na interpretação).**

---

# 3. LLM: o que é?

## Large Language Model

Um LLM é um modelo treinado para trabalhar com sequências de tokens e prever continuações plausíveis.

```mermaid
flowchart LR
    A[Prompt] --> B[Tokens]
    B --> C[LLM]
    C --> D[Tokens de saída]
    D --> E[Resposta]
```

### Para software embarcado

A LLM pode:

- interpretar uma especificação;
- propor arquitetura;
- gerar C/C++/Rust/MicroPython;
- escrever testes;
- analisar logs;
- sugerir drivers;
- revisar código;
- encontrar inconsistências.

**Mas ela não conhece automaticamente seu hardware real.**

---

# 4. Tokens

Um token não é exatamente uma palavra.

Pode ser:

- uma palavra;
- parte de uma palavra;
- pontuação;
- trecho de código;
- espaço ou combinação frequente de caracteres.

Exemplo conceitual:

```text
"HAL_GPIO_WritePin()"
      ↓
   vários tokens
```

A LLM trabalha sobre tokens, não diretamente sobre "palavras".

### Consequência

Quanto mais contexto fornecemos:

- mais tokens de entrada;
- mais processamento;
- potencialmente maior custo;
- maior pressão sobre a janela de contexto.

---

# 5. Janela de contexto

A **janela de contexto** é a quantidade máxima de tokens que o modelo consegue considerar em uma interação.

```mermaid
flowchart LR
    A[System] --> C[Contexto]
    B[Arquivos] --> C
    D[Conversa] --> C
    E[Instruções] --> C
    C --> F[LLM]
    F --> G[Resposta]
```

Para firmware, o contexto pode conter:

- especificação;
- `constitution.md`;
- `TARGET.md`;
- drivers;
- testes;
- logs;
- datasheets;
- decisões de arquitetura.

### Problema

**Mais contexto não significa automaticamente melhor resposta.**

Contexto irrelevante pode:

- aumentar custo;
- reduzir eficiência;
- dificultar localização da informação importante.

---

# 6. Custo dos tokens

Em serviços de LLM, o preço normalmente depende dos tokens processados.

Podemos pensar:

```text
Custo ≈ tokens de entrada × preço de entrada
      + tokens de saída × preço de saída
```

Há ainda diferenças entre:

- modelos;
- provedores;
- cache;
- contexto reutilizado;
- capacidade de raciocínio;
- limites de uso.

## No desenvolvimento embarcado

Um fluxo eficiente tenta:

> **fornecer o contexto necessário, e não todo o repositório indiscriminadamente.**

---

# 7. Execução local × provedor

```mermaid
flowchart LR
    subgraph Local
        A[VS Code] --> B[LLM local]
        B --> C[PC / workstation]
    end

    subgraph Remoto
        D[VS Code] --> E[API / Provedor]
        E --> F[Infraestrutura do provedor]
    end
```

## LLM local

Vantagens:

- dados podem permanecer localmente;
- sem dependência obrigatória de internet;
- controle do modelo;
- possibilidade de custo previsível.

Desvantagens:

- exige hardware;
- modelos maiores podem ser pesados;
- desempenho depende da máquina.

## Provedor

Vantagens:

- modelos muito grandes;
- infraestrutura pronta;
- atualização mais simples.

Desvantagens:

- custo por uso;
- dependência de serviço externo;
- requisitos de política e privacidade.

---

# 8. O que é um Harness?

**Harness = estrutura de suporte para executar, testar, medir e avaliar um sistema de forma controlada.**

No contexto de IA:

```mermaid
flowchart LR
    P[Prompt / tarefa] --> H[Harness]
    H --> L[LLM]
    L --> T[Tools]
    T --> R[Resultado]
    R --> E[Eval / métricas]
    E --> H
```

Um harness pode controlar:

- prompts;
- contexto;
- ferramentas;
- número de passos;
- testes;
- métricas;
- comparação entre modelos;
- critérios de aprovação.

### Analogia embarcada

É semelhante a construir um **ambiente de teste reproduzível** para firmware.

Você não pergunta apenas:

> "Funcionou?"

Você pergunta:

> "Em qual cenário, com qual entrada, com qual versão, com qual medição e com qual evidência?"

---

# 9. Por que Harness importa no embarcado?

Firmware não deve ser avaliado apenas pela qualidade textual do código.

Devemos medir:

- build;
- tamanho de flash;
- RAM;
- tempo de execução;
- latência;
- jitter;
- consumo;
- comportamento após reset;
- comportamento do watchdog;
- comunicação;
- falhas.

```mermaid
flowchart TD
    S[Especificação] --> I[Implementação]
    I --> B[Build]
    B --> T[Testes]
    T --> M[Medições]
    M --> E[Evidência]
    E --> S2[Decisão]
    S2 -->|ajustes| S
```

**O harness fecha o ciclo entre intenção e evidência.**

---

# 10. O que é SDD?

## Specification-Driven Development

A especificação é a fonte de intenção.

O código é uma implementação dessa intenção.

```mermaid
flowchart LR
    A[Requisito] --> B[Especificação]
    B --> C[Design]
    C --> D[Tarefas]
    D --> E[Implementação]
    E --> F[Verificação]
    F --> G[Evidência]
    G --> B
```

### Importante

SDD não significa:

> "escrever muita documentação".

Significa:

> **definir comportamento observável e mantê-lo conectado à implementação e aos testes.**

---

# 11. SDD adaptativo

Nem todo problema merece o mesmo processo.

| Escopo | Exemplo | Processo |
|---|---|---|
| Pequeno | corrigir um bug local | `TASK.md` |
| Médio | novo driver conhecido | `spec.md` |
| Grande | protocolo + RTOS + persistência | `spec + design + tasks` |
| Complexo | nova MCU/placa + timing crítico | especificar → pesquisar → projetar → implementar → validar |

### Regra prática

**Aumente o rigor quando aumentarem risco, dependências ou incerteza.**

---

# 12. Arquivos do SDD

O arquivo `SKILL.md` propõe uma estrutura organizada:

```text
.specs/
├── project/
│   ├── PROJECT.md
│   ├── ROADMAP.md
│   ├── STATE.md
│   └── constitution.md
├── codebase/
│   ├── STACK.md
│   ├── TARGET.md
│   ├── ARCHITECTURE.md
│   ├── CONVENTIONS.md
│   ├── TESTING.md
│   ├── INTEGRATIONS.md
│   └── CONCERNS.md
└── features/<recurso>/
    ├── spec.md
    ├── context.md
    ├── design.md
    └── tasks.md
```

Para correções pequenas:

```text
quick/NNN-slug/
├── TASK.md
└── SUMMARY.md
```

---

# 13. O papel do `constitution.md`

A constituição define princípios estáveis do projeto.

Exemplos:

- segurança;
- estado seguro;
- determinismo;
- concorrência;
- limites de RAM/flash;
- energia;
- interfaces;
- rastreabilidade;
- diagnóstico;
- recuperação;
- gates.

```mermaid
flowchart TD
    C[constitution.md] --> S[spec.md]
    C --> D[design.md]
    C --> T[tasks.md]
    C --> V[verificação]
```

### Ideia-chave

A constituição não descreve cada feature.

Ela define **as regras do jogo**.

---

# 14. O papel do `SKILL.md`

O `SKILL.md` descreve como conduzir o trabalho.

Ele orienta a IA sobre:

- reconhecer o contexto;
- especificar comportamento;
- discutir ambiguidades;
- projetar;
- quebrar em tarefas;
- executar;
- verificar;
- ancorar e encerrar.

### Em outras palavras

`constitution.md`:

> **o que nunca deve ser violado**

`SKILL.md`:

> **como conduzir o processo**

---

# 15. Primeiro conceito de MCU: o microcontrolador

Uma MCU integra vários recursos em um único circuito integrado.

```mermaid
flowchart TB
    MCU[MCU]
    MCU --> CPU[CPU]
    MCU --> RAM[RAM]
    MCU --> FLASH[Flash]
    MCU --> GPIO[GPIO]
    MCU --> TIM[Timers]
    MCU --> ADC[ADC]
    MCU --> COM[Interfaces]
    MCU --> WDG[Watchdog]
    MCU --> INT[Interrupt Controller]
```

A programação de firmware depende diretamente desses recursos.

---

# 16. CPU: mono core × multi core

## Monocore

Um núcleo de processamento.

```text
Core
 ├─ código
 ├─ ISR
 └─ tasks
```

## Multicore

Dois ou mais núcleos.

```mermaid
flowchart LR
    M[MCU] --> C0[Core 0]
    M --> C1[Core 1]
    M --> R[Memória compartilhada]
```

### SDD precisa explicitar

- qual núcleo executa cada tarefa;
- compartilhamento de memória;
- sincronização;
- ownership;
- comunicação entre cores;
- impacto no timing.

---

# 17. Memória: RAM × Flash

## RAM

Normalmente usada para dados durante execução.

Exemplos:

- variáveis;
- stack;
- heap;
- buffers;
- filas.

## Flash

Normalmente usada para:

- firmware;
- constantes;
- configuração persistente;
- dados que precisam sobreviver ao desligamento.

```mermaid
flowchart LR
    P[Power OFF] --> R{RAM}
    R -->|dados perdidos| X[Perda]
    P --> F{Flash}
    F -->|dados preservados| D[Persistência]
```

### Em SDD

RAM e flash são **recursos finitos e mensuráveis**.

---

# 18. Persistência de dados

Persistência significa que o dado sobrevive ao reset ou desligamento.

Exemplos:

- calibração;
- configuração;
- contador;
- parâmetros de controle.

Mas escrever em memória não é "grátis".

Precisamos discutir:

- número de ciclos de escrita;
- integridade;
- CRC;
- versionamento;
- atualização atômica;
- recuperação após perda de energia.

```mermaid
flowchart TD
    A[Novo dado] --> B[Buffer]
    B --> C[Validação]
    C --> D[Gravação]
    D --> E[CRC / versão]
    E --> F[Persistência]
```

---

# 19. GPIO e portas

GPIO = General Purpose Input/Output.

Podem atuar como:

- entrada digital;
- saída digital;
- funções alternativas.

```text
GPIO
 ├─ INPUT
 ├─ OUTPUT
 └─ Alternate Function
```

### SDD não deve escrever apenas:

> "ligar o sensor no pino X".

Deve haver definição de:

- pino;
- nível elétrico;
- polaridade;
- pull-up/pull-down;
- função;
- frequência;
- comportamento após reset.

---

# 20. Interrupções — ISR

ISR = **Interrupt Service Routine**.

Uma interrupção permite reagir a eventos sem depender exclusivamente do loop principal.

```mermaid
sequenceDiagram
    participant HW as Periférico
    participant CPU as CPU
    participant ISR as ISR
    participant TASK as Task/Loop

    HW->>CPU: Evento
    CPU->>ISR: interrompe fluxo
    ISR->>ISR: captura evento
    ISR-->>TASK: sinaliza / enfileira
    TASK->>TASK: processa trabalho
```

## Regra essencial

A ISR deve ser:

- curta;
- determinística;
- não bloqueante;
- livre de trabalho pesado quando possível.

O `constitution.md` explicita essa preocupação.

---

# 21. Timers, frequência e tempo real

Timers permitem construir eventos temporizados.

Exemplos:

- periodicidade de amostragem;
- PWM;
- timeout;
- contagem;
- scheduler.

### Três conceitos

**Período**

```text
T = 1 / f
```

**Deadline**

Tempo máximo para concluir uma atividade.

**Jitter**

Variação do instante real de execução em torno do instante esperado.

---

# 22. Watchdog, reset e brownout

## Watchdog

Monitora se o software continua respondendo.

```mermaid
flowchart TD
    A[Software executa] --> B[Alimenta watchdog]
    B --> A
    A --> C{Travou?}
    C -->|sim| D[Watchdog expira]
    D --> E[Reset]
    E --> F[Recuperação]
```

## Reset

Pode ocorrer por:

- power-on;
- watchdog;
- software;
- brownout;
- periférico;
- pino externo.

### SDD

Falha também é comportamento.

---

# 23. Energia

Em embarcados, energia pode ser requisito funcional.

Exemplos:

- corrente máxima;
- duty cycle;
- modo sleep;
- tempo acordado;
- tempo de inicialização;
- autonomia.

```mermaid
flowchart LR
    A[Ativo] --> B[Idle]
    B --> C[Sleep]
    C --> D[Wake-up]
    D --> A
```

**"Baixo consumo" não é uma especificação.**

Uma especificação deve indicar:

- quanto;
- em qual estado;
- por quanto tempo;
- como medir.

---

# 24. Barramentos e interfaces

### Comunicação interna

- I²C
- SPI
- I²S
- OneWire

### Interfaces digitais / físicas

- GPIO
- PWM
- UART / TTL
- USB
- CAN
- RS-232
- RS-485

### Conversão

- ADC / A/D
- DAC / D/A

Cada interface possui:

- timing;
- níveis elétricos;
- framing;
- erros;
- limites;
- compatibilidade.

---

# 25. TTL não é um protocolo

Esse é um ponto importante.

**TTL** descreve níveis elétricos/lógicos.

**UART** descreve um método de comunicação serial assíncrona.

Portanto:

```text
UART + níveis TTL
```

é diferente de:

```text
UART + transceptor RS-485
```

### Por que isso importa?

Uma LLM pode gerar código UART correto e ainda assim o sistema físico estar errado.

**Software correto não compensa interface elétrica incorreta.**

---

# 26. Interfaces: visão de engenharia

```mermaid
flowchart LR
    A[Aplicação] --> B[Driver]
    B --> C[HAL / SDK]
    C --> D[Periférico MCU]
    D --> E[Pinagem]
    E --> F[Interface elétrica]
    F --> G[Dispositivo externo]
```

SDD deve registrar o caminho completo.

Exemplo:

```text
FR-012
O firmware deve adquirir temperatura a 10 Hz.

NFR-007
O intervalo entre amostras deve ter jitter <= X.

Design
Timer → ISR → fila → task → driver → ADC/sensor.
```

---

# 27. RTOS

Um RTOS organiza execução concorrente em tarefas/threads.

```mermaid
flowchart TB
    RTOS[RTOS]
    RTOS --> T1[Task Sensor]
    RTOS --> T2[Task Comunicação]
    RTOS --> T3[Task Controle]
    RTOS --> ISR[ISR]
    T1 --> Q[Fila]
    T2 --> Q
    ISR --> Q
```

Conceitos importantes:

- task/thread;
- prioridade;
- scheduler;
- fila;
- semáforo;
- mutex;
- evento;
- timeout;
- stack.

### SDD

Descrever apenas "crie uma task" é insuficiente.

É necessário definir:

- finalidade;
- período;
- prioridade;
- deadline;
- recursos compartilhados;
- ownership;
- política de bloqueio.

---

# 28. Máquina de estados

Muitos sistemas embarcados são naturalmente descritos como máquinas de estados.

```mermaid
stateDiagram-v2
    [*] --> INIT
    INIT --> READY: inicialização OK
    INIT --> FAULT: erro
    READY --> RUN: comando
    RUN --> READY: parada
    RUN --> FAULT: erro crítico
    FAULT --> INIT: reset/recovery
```

SDD deve tornar explícitos:

- estados;
- eventos;
- transições;
- ações;
- condições de erro;
- estado seguro.

---

# 29. Especificação: comportamento observável

Um requisito bom é testável.

### Ruim

> "O sistema deve ser rápido."

### Melhor

> `NFR-003`: O comando deve ser processado em até 10 ms, medidos da recepção válida até a atualização da saída.

### Critério de aceitação

```text
DADO que o sistema esteja em READY
QUANDO receber um comando válido
ENTÃO deve executar a transição para RUN
E atualizar a saída em até 10 ms.
```

---

# 30. FR × NFR

## FR — Functional Requirement

Descreve **o que o sistema faz**.

Exemplo:

```text
FR-021
O firmware deve amostrar o ADC periodicamente.
```

## NFR — Non-Functional Requirement

Descreve restrições mensuráveis.

```text
NFR-021
A frequência de amostragem deve ser 1 kHz ± 1%.
```

Outros NFR:

- latência;
- jitter;
- RAM;
- flash;
- energia;
- disponibilidade;
- segurança;
- diagnóstico.

---

# 31. Design não é a mesma coisa que requisito

```mermaid
flowchart LR
    A[Requisito] --> B["O que"]
    B --> C[Design]
    C --> D["Como"]
    D --> E[Implementação]
```

Exemplo:

**Requisito**

> O sensor deve ser amostrado a 100 Hz.

**Design**

> Timer periódico gera evento; ISR coloca timestamp em fila; task realiza aquisição e validação.

O requisito permanece independente da implementação.

---

# 32. E o hardware?

## Aqui está uma fronteira importante

**Esta palestra trata do software embarcado.**

Ela **não substitui**:

- especificação elétrica;
- seleção do MCU;
- seleção de componentes;
- projeto esquemático;
- layout PCB;
- integridade de sinal;
- EMC/EMI;
- dissipação térmica;
- fonte de alimentação;
- análise de tolerâncias;
- segurança elétrica.

### SDD deve conhecer o hardware

Mas isso é diferente de afirmar:

> "A IA pode projetar sozinha o hardware."

---

# 33. A ponte entre hardware e software

```mermaid
flowchart LR
    HW[Especificação e projeto do hardware]
    HW --> IF[Interface elétrica]
    IF --> FW[Especificação do firmware]
    FW --> DRV[Drivers]
    DRV --> APP[Aplicação]
    APP --> TEST[Testes]
    TEST --> HIL[HIL / Hardware real]
    HIL --> HW
```

### Regra

Antes de implementar:

> **confirme MCU, placa, clock, alimentação, pinos, periféricos, toolchain, SDK e RTOS.**

O `SKILL.md` chama atenção explícita para não inventar detalhes de hardware.

---

# 34. O arquivo `TARGET.md`

Uma boa ponte entre hardware e software é documentar o alvo.

Exemplo conceitual:

```markdown
# TARGET

- MCU: [fabricante/família/part number]
- Board: [placa/revisão]
- Clock: [frequência]
- Alimentação: [tensão]
- Flash: [capacidade]
- RAM: [capacidade]
- GPIO: [quantidade]
- ADC: [resolução/canais]
- Timers: [quantidade]
- Interfaces: [SPI/I2C/UART/USB/...]
- RTOS: [versão]
- SDK: [versão]
- Toolchain: [versão]
```

### Se não souber

Não invente.

Escreva:

```text
A CONFIRMAR
```

---

# 35. Verificação em níveis

O `SKILL.md` diferencia níveis de evidência:

```mermaid
flowchart LR
    H[HOST] --> S[SIMULADOR]
    S --> B[BANCADA]
    B --> HIL[HIL]
```

## HOST

Lógica pura, parser, cálculo, máquina de estados.

## SIMULADOR

Periféricos e temporização modelados.

## BANCADA

Pinos, níveis, sensores, atuadores, reset, watchdog e consumo.

## HIL

Integração com hardware real, timing, comunicação e falhas.

---

# 36. Uma armadilha comum

> "O teste passou no computador, então o firmware está validado."

**Não necessariamente.**

Um teste HOST não prova:

- timing real;
- comportamento da ISR;
- níveis elétricos;
- ruído;
- latência física;
- reset;
- brownout;
- watchdog;
- consumo;
- interação com periféricos reais.

O documento define explicitamente que hardware não deve ser declarado validado apenas por testes HOST.

---

# 37. Rastreabilidade

```mermaid
flowchart TD
    R1[FR-001] --> D1[Design]
    R1 --> T1[Teste]
    T1 --> E1[Evidência]

    R2[NFR-004] --> D2[Design]
    R2 --> T2[Teste]
    T2 --> E2[Medição]
```

A pergunta final é:

> **Consigo apontar para cada requisito e mostrar como ele foi implementado e como foi verificado?**

---

# 38. Exemplo completo: botão + LED

## Requisito

```text
FR-001
Ao pressionar o botão, o LED deve alternar seu estado.

NFR-001
O evento deve ser reconhecido entre 1 ms e 10 ms.

NFR-002
Ruído mecânico não deve gerar múltiplas transições.
```

## Design

```text
GPIO interrupt
      ↓
ISR curta
      ↓
timestamp/evento
      ↓
task/loop
      ↓
debounce
      ↓
toggle LED
```

---

# 39. Exemplo: aquisição de sensor

```mermaid
sequenceDiagram
    participant TIMER as Timer
    participant ISR as ISR
    participant Q as Queue
    participant TASK as Sensor Task
    participant ADC as ADC

    TIMER->>ISR: período
    ISR->>Q: evento
    Q->>TASK: sinal
    TASK->>ADC: inicia aquisição
    ADC-->>TASK: valor
    TASK->>TASK: valida / filtra
    TASK->>TASK: registra timestamp
```

Perguntas SDD:

- Qual frequência?
- Qual resolução?
- Qual faixa?
- Qual tempo de estabilização?
- Qual erro aceitável?
- O que acontece fora da faixa?
- O que acontece se o ADC falhar?
- Onde armazenar os dados?

---

# 40. Como usar o VS Code

A ideia é transformar o repositório em um **ambiente de engenharia**.

```text
projeto/
├── .specs/
│   ├── project/
│   ├── codebase/
│   ├── features/
│   └── quick/
├── src/
├── include/
├── tests/
├── docs/
└── .git/
```

No VS Code:

1. abra o repositório;
2. mantenha a especificação próxima ao código;
3. use o `constitution.md` como regra estável;
4. use o processo do `SKILL.md`;
5. peça à IA para trabalhar sobre uma tarefa específica;
6. revise mudanças;
7. execute os gates;
8. registre a evidência.

---

# 41. Fluxo de uma feature no VS Code

```mermaid
flowchart TD
    A[Ideia / necessidade] --> B[classificar risco]
    B --> C{Escopo}
    C -->|Pequeno| D[TASK.md]
    C -->|Médio| E[spec.md]
    C -->|Grande| F[spec + design + tasks]
    C -->|Complexo| G[especificar + pesquisar]
    D --> H[Implementar]
    E --> H
    F --> H
    G --> I[Projetar]
    I --> H
    H --> J[Build]
    J --> K[Testes]
    K --> L[Validação]
    L --> M[Evidence / SUMMARY]
```

---

# 42. Demonstração prática da palestra

## Cenário

> "Adicionar leitura de temperatura a cada 1 segundo e enviar a leitura pela UART."

### A IA não deve começar gerando código.

Primeiro:

**Perguntar / verificar:**

- qual MCU?
- qual sensor?
- qual interface?
- qual baud rate?
- qual faixa?
- qual unidade?
- qual precisão?
- qual timeout?
- qual comportamento de erro?
- qual RTOS?
- qual pino?

---

# 43. Da demanda para a especificação

Exemplo de `spec.md`:

```markdown
# Leitura de temperatura

## Objetivo
Adquirir temperatura e transmitir o valor pela UART.

## FR-001
O firmware deve adquirir temperatura a cada 1 s.

## FR-002
O valor deve ser transmitido pela UART após aquisição válida.

## NFR-001
O período deve ser 1 s ± 10 ms.

## FR-003
Uma leitura inválida deve gerar código de erro.

## Critérios de aceitação

DADO o sistema em operação
QUANDO ocorrer o período de aquisição
ENTÃO uma leitura deve ser realizada.

DADO uma leitura válida
QUANDO a aquisição terminar
ENTÃO o valor deve ser transmitido.
```

---

# 44. Depois vem o design

```markdown
# Design

Timer 1 s
  -> ISR
  -> fila/evento
  -> task de aquisição
  -> driver do sensor
  -> validação
  -> formatter
  -> driver UART
```

### O design responde

- onde executa?
- quem acessa o periférico?
- como dados trafegam?
- qual contexto é ISR?
- qual contexto é task?
- como tratar concorrência?
- qual memória é usada?

---

# 45. Depois vêm as tarefas

Exemplo:

```text
T-001 Criar contrato do driver do sensor
T-002 Implementar timer periódico
T-003 Implementar aquisição
T-004 Implementar validação
T-005 Implementar transmissão UART
T-006 Criar testes HOST
T-007 Executar build do alvo
T-008 Validar na bancada
```

Cada tarefa deve possuir:

- requisitos;
- onde alterar;
- dependências;
- condição de conclusão;
- testes;
- gate.

---

# 46. O que a IA deve receber?

Uma solicitação melhor:

```text
Leia:
- .specs/project/constitution.md
- .specs/codebase/TARGET.md
- .specs/features/temperature/spec.md
- .specs/features/temperature/design.md

Implemente T-003.

Não altere:
- pinagem
- protocolo UART
- contrato do driver

Ao terminar:
1. execute o gate definido;
2. informe arquivos alterados;
3. informe testes;
4. informe desvios da especificação.
```

Isso é muito diferente de:

> "Crie um código para ler temperatura."

---

# 47. SDD + LLM = engenharia com contrato

```mermaid
flowchart LR
    U[Engenheiro] --> S[Especificação]
    S --> L[LLM]
    L --> C[Código]
    C --> V[Verificação]
    V --> E[Evidência]
    E --> U
```

A LLM é um **acelerador de trabalho**.

Ela não substitui:

- decisão de engenharia;
- validação;
- análise de risco;
- conhecimento do hardware;
- medição física;
- responsabilidade técnica.

---

# 48. O que a IA não deve inventar

Segundo o processo proposto:

**Não inventar:**

- MCU;
- placa;
- clock;
- alimentação;
- pinos;
- níveis elétricos;
- periféricos;
- toolchain;
- SDK;
- RTOS;
- protocolo.

Quando faltar informação:

```text
A CONFIRMAR
```

ou uma pergunta bloqueadora.

---

# 49. Segurança e falhas

Em software tradicional, podemos pensar principalmente em:

```text
entrada → processamento → saída
```

No embarcado, precisamos também pensar:

```text
entrada
  ↓
processamento
  ↓
saída
  ↓
falha
  ↓
reset / watchdog / brownout
  ↓
recuperação
  ↓
estado seguro
```

### Falha é comportamento.

Essa é uma das ideias mais importantes da palestra.

---

# 50. O que deve entrar na constituição?

Exemplos:

### Determinismo

- período;
- deadline;
- jitter;
- política de alocação.

### Recursos

- RAM;
- flash;
- stack;
- energia.

### Hardware

- pinos;
- polaridade;
- níveis;
- unidades;
- escalas.

### Comunicação

- framing;
- endianess;
- CRC;
- timeout;
- retries;
- versão.

---

# 51. Gates

Antes de considerar uma tarefa concluída:

```text
[ ] requisitos observáveis
[ ] alvo confirmado
[ ] toolchain/SDK confirmados
[ ] reset considerado
[ ] watchdog considerado
[ ] timing considerado
[ ] RAM/flash considerados
[ ] energia considerada
[ ] testes executados no nível correto
[ ] resultado registrado
```

### O gate impede o clássico:

> "Compilou, então está pronto."

---

# 52. A diferença entre gerar código e desenvolver firmware

```mermaid
flowchart TD
    A[Gerar código] --> B[Compilar]
    B --> C[Executar]
    C --> D{Funcionou?}

    E[Desenvolver firmware] --> F[Especificar]
    F --> G[Projetar]
    G --> H[Implementar]
    H --> I[Testar]
    I --> J[Medir]
    J --> K[Validar]
    K --> L[Registrar]
```

A segunda abordagem produz **evidência**.

---

# 53. Encerramento: perguntas que sempre devemos fazer

Antes de pedir código:

1. **O que o sistema deve fazer?**
2. **Em quais condições?**
3. **Com quais limites?**
4. **Qual hardware executará isso?**
5. **Qual timing é necessário?**
6. **O que acontece quando falha?**
7. **Como será testado?**
8. **Que evidência provará que está correto?**

---

# 54. Mensagem final

## SDD não é "documentação antes do código".

É uma forma de manter alinhados:

```text
INTENÇÃO
   ↓
ESPECIFICAÇÃO
   ↓
DESIGN
   ↓
IMPLEMENTAÇÃO
   ↓
TESTE
   ↓
EVIDÊNCIA
```

### E, em sistemas embarcados:

> **software, hardware, tempo, memória, energia e falhas fazem parte do problema.**

A IA pode acelerar a engenharia.

**Mas a engenharia continua sendo responsabilidade humana.**

---

# 55. Roteiro do apresentador

## Tempo sugerido: 90–120 min

### Bloco 1 — IA e ferramentas (15–20 min)
Slides 3–10

Enfatizar:

- tokens;
- contexto;
- custo;
- local × remoto;
- harness.

### Bloco 2 — SDD (20–25 min)
Slides 11–18

Mostrar no VS Code:

- `SKILL.md`;
- `constitution.md`;
- `.specs/`;
- rastreabilidade.

### Bloco 3 — Fundamentos de MCU (25–35 min)
Slides 15–29

Conectar:

- GPIO;
- ISR;
- timers;
- memória;
- interfaces;
- RTOS;
- watchdog;
- energia.

### Bloco 4 — Aplicação prática (20–30 min)
Slides 35–47

Fazer uma demonstração ao vivo:

> "Adicionar leitura de temperatura e transmitir pela UART."

---

# 56. Demonstração no VS Code — roteiro

### Etapa 1

Abrir:

```text
SKILL.md
constitution.md
```

Explicar que são as regras do processo.

### Etapa 2

Mostrar:

```text
.specs/project/
.specs/codebase/
.specs/features/
```

### Etapa 3

Criar uma feature:

```text
.specs/features/temperature/spec.md
```

### Etapa 4

Pedir à IA:

> "Antes de implementar, leia a constituição, o TARGET e a especificação. Liste lacunas bloqueadoras."

### Etapa 5

Completar as informações de hardware.

### Etapa 6

Criar `design.md`.

### Etapa 7

Criar `tasks.md`.

### Etapa 8

Implementar uma tarefa por vez.

### Etapa 9

Executar build/testes.

### Etapa 10

Registrar `SUMMARY.md`.

---

# 57. Pergunta para a turma

## Imagine esta solicitação:

> "Crie um firmware para ler um sensor a cada 100 ms usando interrupção e enviar os dados por USB."

### O que está faltando?

Possíveis respostas:

- MCU;
- sensor;
- interface do sensor;
- pinagem;
- níveis elétricos;
- tipo de USB;
- velocidade/configuração;
- formato do pacote;
- frequência real;
- precisão;
- timeout;
- comportamento de falha;
- RTOS;
- uso de RAM;
- critério de teste.

**Se a turma perceber isso, o objetivo central da palestra foi atingido.**

---

# 58. Referência dos arquivos utilizados

### `SKILL.md`

O documento define o processo de SDD para software embarcado, incluindo princípios, artefatos, dimensionamento adaptativo, especificação, design, tarefas, execução, verificação e rastreabilidade.

### `constitution.md`

O documento define princípios estáveis para um projeto embarcado: segurança, determinismo, recursos, interfaces, qualidade, diagnóstico, recuperação, processo de mudança e gates.

> Os dois arquivos devem ser tratados como exemplos de uma base de engenharia que pode ser adaptada ao projeto real.

---

# 59. Última mensagem

## Não pergunte apenas:

> "Qual código a IA deve escrever?"

Pergunte:

> **"Qual comportamento devo especificar, quais restrições devem ser respeitadas e qual evidência provará que o firmware funciona no hardware real?"**

---
