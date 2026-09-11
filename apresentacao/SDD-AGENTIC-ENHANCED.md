# SDD Agentic Enhanced

## Visão geral

O **SDD Agentic Enhanced** é uma extensão de uma estrutura de **Spec-Driven Development (SDD)** para desenvolvimento agêntico de sistemas embarcados.

A proposta é combinar:

- especificação orientada a requisitos;
- agentes especializados;
- Rules/Instructions permanentes;
- economia rigorosa de contexto;
- roteamento adaptativo de tarefas;
- rastreabilidade entre requisito, código, teste e evidência;
- gates de verificação;
- handoff entre agentes e sessões;
- critérios objetivos para declarar uma tarefa concluída.

O objetivo não é simplesmente aumentar a quantidade de agentes. O objetivo é fazer com que o sistema utilize **o menor número de agentes e o menor volume de contexto necessários para executar cada tarefa com segurança e rastreabilidade**.

---

## 1. Problema que o SDD Agentic Enhanced resolve

Um conjunto tradicional de agentes pode acabar seguindo sempre um fluxo semelhante a:

```text
Planner
   ↓
Requirements
   ↓
Architect
   ↓
Task Planner
   ↓
Implementer
   ↓
Tester
   ↓
Reviewer
   ↓
Security
   ↓
Documentation
   ↓
Integration
   ↓
Release
```

Esse fluxo é adequado para funcionalidades grandes, mas é excessivo para alterações pequenas.

Por exemplo:

> Corrigir uma conversão de temperatura.

Não é eficiente executar todo o processo arquitetural para uma mudança localizada.

O SDD Agentic Enhanced transforma o fluxo em um processo **adaptativo**:

```text
                         Solicitação
                              │
                              ▼
                        Task Router
                              │
                    ┌─────────┼─────────┐
                    │         │         │
                    ▼         ▼         ▼
                  Baixo      Médio     Alto
                    │         │         │
                    ▼         ▼         ▼
                Quick Fix    SDD     Arquitetura
                    │         │         │
                    └─────────┴─────────┘
                              │
                              ▼
                       Implementação
                              │
                              ▼
                         Verificação
                              │
                              ▼
                           Evidência
```

---

# 2. Princípios fundamentais

## 2.1. Contexto não é memória confiável

O histórico da conversa pode informar a intenção do usuário, mas não deve ser considerado a fonte de verdade sobre o estado atual do código.

A fonte de verdade é:

1. workspace atual;
2. estado do Git;
3. Constituição;
4. especificação;
5. design/ADR;
6. tarefas;
7. código e testes;
8. evidências.

Portanto, antes de modificar código, o agente deve verificar o estado real do workspace.

---

## 2.2. Economia de contexto

O agente não deve explorar todo o repositório para "entender o projeto".

Deve seguir:

```text
Tarefa
  ↓
Identificar arquivos necessários
  ↓
Ler somente o necessário
  ↓
Executar a tarefa
  ↓
Descartar contexto não necessário
```

Evitar:

```bash
find .
tree
ls -R
cat dezenas de arquivos
```

quando esses comandos não forem necessários.

---

## 2.3. Evidência antes de conclusão

Uma tarefa não é considerada concluída simplesmente porque:

- o código foi escrito;
- o agente afirma que funciona;
- o código compila.

O sistema deve buscar evidências.

Exemplo:

```text
FR-001
   ↓
CA-001
   ↓
T-003
   ↓
src/temperature.cpp
   ↓
test_temperature.cpp
   ↓
PASS
```

Se não for possível testar no hardware:

```text
CA-003 → PENDENTE
Motivo: hardware não disponível.
Risco: validação física ainda necessária.
```

Nunca transformar uma validação não realizada em `PASS`.

---

# 3. Estrutura do pacote

A estrutura recomendada é:

```text
.github/
├── agents/
│   ├── 00-task-router.agent.md
│   ├── 01-product-planner.agent.md
│   ├── 02-requirements-engineer.agent.md
│   ├── 03-architect.agent.md
│   ├── 04-task-planner.agent.md
│   ├── 05-implementer.agent.md
│   ├── 06-code-reviewer.agent.md
│   ├── 07-test-engineer.agent.md
│   ├── 08-security-reviewer.agent.md
│   ├── 09-documentation.agent.md
│   ├── 10-integration-agent.agent.md
│   ├── 11-release-agent.agent.md
│   ├── 12-project-orchestrator.agent.md
│   ├── 13-verification-gate.agent.md
│   └── 14-handoff-manager.agent.md
│
└── instructions/
    ├── 00-context-economy.instructions.md
    ├── 01-embedded-engineering.instructions.md
    ├── 02-sdd-artifacts.instructions.md
    └── 03-change-control.instructions.md

.specify/
└── memory/
    └── constitution.md

docs/
└── agentic/
    ├── WORKFLOW.md
    ├── TRACEABILITY.md
    ├── DEFINITION-OF-DONE.md
    ├── AGENT-CONTEXT-PROTOCOL.md
    └── AGENT-OPERATING-MODE.md
```

Os agentes existentes do projeto original são preservados. Os novos artefatos acrescentam uma camada de controle.

---

# 4. Agentes

## 4.1. Task Router

Arquivo:

```text
.github/agents/00-task-router.agent.md
```

Responsabilidade:

- classificar a solicitação;
- avaliar complexidade;
- avaliar risco;
- escolher o menor fluxo adequado;
- encaminhar para o agente especializado.

Categorias:

```text
PERGUNTA
QUICK FIX
MUDANÇA PEQUENA
MUDANÇA MÉDIA
MUDANÇA GRANDE
VALIDAÇÃO
DOCUMENTAÇÃO
RELEASE
```

Níveis de risco:

```text
BAIXO
MÉDIO
ALTO
CRÍTICO
```

O Task Router não deve implementar a solução.

---

## 4.2. Agentes especializados

Os agentes especializados continuam sendo responsáveis por seus respectivos domínios:

- planejamento;
- requisitos;
- arquitetura;
- decomposição em tarefas;
- implementação;
- testes;
- revisão;
- segurança;
- documentação;
- integração;
- release.

A melhoria principal é que eles não precisam ser acionados sempre.

---

## 4.3. Verification Gate

Arquivo:

```text
.github/agents/13-verification-gate.agent.md
```

É responsável por responder:

> Existe evidência suficiente para declarar a tarefa concluída?

Cada critério recebe:

```text
PASS
FAIL
PENDENTE
```

O agente deve considerar:

- requisitos;
- critérios de aceitação;
- build;
- testes;
- análise estática;
- RAM/Flash/Stack;
- timing;
- documentação;
- Git;
- riscos residuais;
- validação física.

---

## 4.4. Handoff Manager

Arquivo:

```text
.github/agents/14-handoff-manager.agent.md
```

É utilizado quando o trabalho precisa continuar em outra sessão ou por outro agente.

O handoff deve conter:

```text
Objetivo
Estado atual
Arquivos modificados
Decisões
Testes/evidências
Problemas
Pendências
Próximo passo
Riscos
Critério de conclusão
```

Não deve copiar grandes quantidades de código ou logs.

---

# 5. Rules / Instructions

## 5.1. Economia de contexto

Arquivo:

```text
.github/instructions/00-context-economy.instructions.md
```

Regras principais:

1. Não fazer varredura proativa.
2. Identificar primeiro a tarefa.
3. Ler somente os arquivos necessários.
4. Preferir leituras direcionadas.
5. Evitar dumps grandes.
6. Executar Git antes de modificar.
7. Não confiar no histórico da conversa para determinar o estado atual.
8. Limpar mentalmente o contexto após a conclusão.

Antes de alterações:

```bash
git status --short
git diff --stat
git diff
```

---

## 5.2. Engenharia embarcada

Arquivo:

```text
.github/instructions/01-embedded-engineering.instructions.md
```

Reforça cuidados com:

- MCU;
- periféricos;
- pinagem;
- registradores;
- ISR;
- DMA;
- `volatile`;
- atomicidade;
- concorrência;
- HAL/BSP;
- watchdog;
- brownout;
- reset;
- timeout;
- RAM;
- Flash;
- stack;
- CPU;
- timing;
- jitter;
- energia;
- largura de banda.

Quando uma informação de hardware estiver ausente, o agente deve indicar:

```text
A CONFIRMAR
```

e não inventar uma característica.

---

## 5.3. Artefatos SDD

Arquivo:

```text
.github/instructions/02-sdd-artifacts.instructions.md
```

Padroniza identificadores:

| Prefixo | Significado |
|---|---|
| `FR-###` | Requisito funcional |
| `NFR-###` | Requisito não funcional |
| `CA-###` | Critério de aceitação |
| `T-###` | Tarefa |
| `ADR-###` | Decisão arquitetural |

A rastreabilidade recomendada é:

```text
Requisito
   ↓
Critério de aceitação
   ↓
Tarefa
   ↓
Código
   ↓
Teste
   ↓
Evidência
```

---

## 5.4. Controle de mudanças

Arquivo:

```text
.github/instructions/03-change-control.instructions.md
```

Antes:

1. definir escopo;
2. definir fora de escopo;
3. identificar arquivos necessários;
4. executar `git diff`;
5. definir o gate.

Durante:

- alterar o mínimo necessário;
- evitar refatoração não relacionada;
- não alterar testes somente para fazê-los passar;
- justificar novas dependências;
- parar diante de mudança arquitetural inesperada.

Depois:

1. compilar;
2. testar;
3. executar análise estática quando aplicável;
4. verificar critérios;
5. atualizar documentação;
6. registrar riscos.

---

# 6. Artefatos de engenharia

## 6.1. WORKFLOW.md

Define o fluxo adaptativo:

```text
Solicitação
    ↓
Task Router
    ↓
Complexidade/Risco
    ├── Pergunta
    ├── Quick Fix
    ├── Médio
    └── Grande
```

Também define os gates:

```text
Gate 0 — Estado
Gate 1 — Intenção
Gate 2 — Design
Gate 3 — Implementação
Gate 4 — Evidência
Gate 5 — Encerramento
```

---

## 6.2. TRACEABILITY.md

Define a matriz:

| Requisito | Critério | Tarefa | Código | Teste | Evidência | Estado |
|---|---|---|---|---|---|---|
| FR-001 | CA-001 | T-001 | `src/...` | `test_...` | build | PASS |
| NFR-001 | CA-002 | T-002 | `src/...` | `test_...` | medição | PENDENTE |

Essa matriz é particularmente útil para projetos acadêmicos e industriais.

---

## 6.3. DEFINITION-OF-DONE.md

Define quando uma tarefa realmente pode ser encerrada.

Exemplo:

```text
[ ] Requisito identificado
[ ] Critérios verificados
[ ] git diff executado
[ ] Implementação realizada
[ ] Build realizado
[ ] Testes realizados
[ ] Análise estática realizada
[ ] Recursos verificados
[ ] Documentação atualizada
[ ] Rastreabilidade atualizada
[ ] Riscos registrados
```

---

## 6.4. AGENT-CONTEXT-PROTOCOL.md

Define como o contexto deve ser transferido entre agentes.

A prioridade é:

```text
Workspace/Git
    ↓
Constitution
    ↓
Specification
    ↓
Design/ADR
    ↓
Tasks
    ↓
Código/Testes
    ↓
Evidências
    ↓
STATUS/HANDOFF
    ↓
Histórico da conversa
```

O histórico da conversa é deliberadamente colocado no final.

---

## 6.5. AGENT-OPERATING-MODE.md

Define comportamentos gerais:

- planejar antes de editar;
- verificar antes de assumir;
- implementar pouco por vez;
- testar imediatamente;
- registrar decisões;
- não fabricar evidências;
- declarar incertezas.

Também define **Stop Conditions**.

O agente deve parar quando:

- faltar informação crítica de hardware;
- houver conflito entre especificações;
- for necessária mudança arquitetural não aprovada;
- houver risco de perda de dados;
- houver impacto de segurança não especificado;
- houver validação obrigatória que não possa ser realizada.

---

# 7. Fluxos recomendados

## 7.1. Pergunta simples

Exemplo:

> O que significa debounce?

Fluxo:

```text
Task Router
    ↓
Resposta
```

Não criar Specification nem executar agentes de implementação.

---

## 7.2. Quick Fix

Exemplo:

> Corrija o cálculo de conversão de °C para °F.

Fluxo:

```text
Task Router
    ↓
git diff
    ↓
Implementer
    ↓
Verification Gate
```

---

## 7.3. Feature média

Exemplo:

> Adicionar suporte a um novo sensor.

Fluxo:

```text
Task Router
    ↓
Requirements
    ↓
Implementer
    ↓
Test Engineer
    ↓
Code Reviewer
    ↓
Verification Gate
```

---

## 7.4. Feature grande

Exemplo:

> Criar um novo subsistema de comunicação Modbus.

Fluxo:

```text
Task Router
    ↓
Requirements
    ↓
Architect
    ↓
Task Planner
    ↓
Implementer
    ↓
Test Engineer
    ↓
Code Reviewer
    ↓
Security
    ↓
Verification
    ↓
Documentation
    ↓
Integration
```

---

## 7.5. Release

Fluxo:

```text
Verification
    ↓
Security
    ↓
Documentation
    ↓
Integration
    ↓
Release
```

---

# 8. Sistemas embarcados

O fluxo foi pensado especialmente para projetos em que software e hardware estão acoplados.

Exemplos de aspectos que devem ser tratados quando aplicáveis:

```text
MCU
Pinagem
Clock
ADC
PWM
UART
SPI
I2C
CAN
Modbus
DMA
ISR
RTOS
Watchdog
Brownout
Bootloader
OTA
Flash
EEPROM
NVS
RAM
Stack
Timing
Jitter
Consumo
Temperatura
EMI/EMC
```

O agente não deve assumir que uma implementação HOST/SIMULADOR comprova automaticamente o comportamento no hardware real.

---

# 9. Níveis de validação

Recomenda-se distinguir:

```text
HOST
  ↓
SIMULADOR
  ↓
BANCADA
  ↓
HIL
  ↓
SISTEMA REAL
```

Um teste em HOST pode provar uma propriedade da lógica.

Ele não necessariamente prova:

- timing real;
- comportamento elétrico;
- ruído;
- interrupções reais;
- DMA;
- consumo;
- comportamento do periférico;
- interação com o hardware.

Por isso a evidência deve indicar claramente o ambiente em que foi obtida.

---

# 10. Exemplo completo

Suponha:

> "Adicionar leitura de temperatura Pt100."

O Task Router pode classificar:

```text
Tipo: Feature média
Risco: Médio/Alto
```

O Requirements Agent cria:

```text
FR-001
O sistema deve adquirir a temperatura do Pt100.

NFR-001
A aquisição deve possuir erro máximo de X °C.

CA-001
Temperatura dentro da faixa especificada deve ser convertida corretamente.

CA-002
Falha do sensor deve ser detectada.
```

O Task Planner cria:

```text
T-001 — Driver
T-002 — Conversão
T-003 — Detecção de falha
T-004 — Testes
```

Implementer altera:

```text
src/pt100.cpp
src/pt100.h
```

Test Engineer cria:

```text
test_pt100.cpp
```

Verification Gate verifica:

```text
FR-001 → PASS
CA-001 → PASS
CA-002 → PASS
NFR-001 → PENDENTE
```

Se a precisão depender de ensaio físico:

```text
PENDENTE — ensaio em bancada necessário.
```

Isso é muito mais confiável do que simplesmente declarar:

```text
"Implementação concluída."
```

---

# 11. Relação com a Constitution

Recomenda-se manter a Constituição enxuta.

## Constitution

Deve conter princípios permanentes, como:

- segurança;
- qualidade;
- rastreabilidade;
- requisitos verificáveis;
- comportamento seguro;
- princípios arquiteturais.

## Rules

Devem conter comportamento operacional:

- economia de contexto;
- uso do Git;
- formato de artefatos;
- regras de firmware;
- controle de mudanças.

## Agents

Devem conter responsabilidades especializadas.

## Tasks

Devem conter o trabalho concreto.

A separação é:

```text
CONSTITUTION
      │
      │ princípios
      ▼
RULES
      │
      │ comportamento
      ▼
AGENTS
      │
      │ especialização
      ▼
TASK
      │
      │ execução
      ▼
CODE + TESTS
      │
      ▼
EVIDENCE
```

---

# 12. Instalação

Copie para a raiz do projeto:

```text
.github/agents/
.github/instructions/
docs/agentic/
```

Mantenha ou integre os agentes existentes.

A Constituição deve permanecer em:

```text
.specify/memory/constitution.md
```

Depois revise a Constituição para evitar duplicação desnecessária com as Rules.

---

# 13. O que não fazer

Não transformar todas as tarefas em um fluxo completo.

Não criar agentes para cada pequena atividade.

Não carregar o repositório inteiro em toda tarefa.

Não confiar no histórico da conversa para determinar o estado atual do código.

Não marcar testes como `PASS` sem evidência.

Não declarar validação física quando somente o HOST foi testado.

Não fazer grandes refatorações durante uma correção localizada.

Não esconder incertezas de hardware.

---

# 14. Benefícios esperados

O SDD Agentic Enhanced busca:

### Menor consumo de contexto

Somente o contexto necessário é carregado.

### Menor latência

Tarefas simples não passam por todos os agentes.

### Maior segurança

Existem regras específicas para sistemas embarcados.

### Maior rastreabilidade

Requisitos podem ser ligados a critérios, tarefas, código, testes e evidências.

### Maior confiabilidade

O Verification Gate impede conclusões sem evidência suficiente.

### Melhor continuidade

O Handoff Manager permite continuar o trabalho sem reconstruir todo o contexto.

### Melhor escalabilidade

O mesmo sistema pode atender:

```text
Quick Fix
   ↓
Feature
   ↓
Subsystem
   ↓
Projeto completo
```

---

# 15. Filosofia central

O principal objetivo deste pacote pode ser resumido em:

> **Não adicionar agentes por adicionar agentes. Adicionar inteligência ao fluxo para que cada tarefa utilize exatamente o nível de processo necessário.**

O sistema ideal deve ser:

```text
                    ┌─────────────────┐
                    │   Solicitação   │
                    └────────┬────────┘
                             ↓
                    ┌─────────────────┐
                    │   Task Router   │
                    └────────┬────────┘
                             ↓
                  ┌──────────────────────┐
                  │ Complexidade + Risco │
                  └──────────┬───────────┘
                             ↓
              ┌──────────────┼──────────────┐
              ↓              ↓              ↓
          QUICK FIX        MÉDIO          GRANDE
              │              │              │
              └──────────────┼──────────────┘
                             ↓
                     IMPLEMENTAÇÃO
                             ↓
                         TESTES
                             ↓
                     VERIFICATION
                             ↓
                        EVIDÊNCIA
                             ↓
                       ENCERRAMENTO
```

O resultado esperado é um sistema **SDD orientado por risco, econômico em contexto, rastreável e adequado ao desenvolvimento agêntico de sistemas embarcados**.
