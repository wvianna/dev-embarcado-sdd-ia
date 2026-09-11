---
name: sdd-embarcado
description: 'Planejar e implementar software embarcado em microcontroladores com SDD adaptativo. Use para especificar firmware, drivers, RTOS, máquinas de estados, interfaces de sensores/atuadores, controle, comunicação, boot, baixo consumo, testes host/HIL e correções de bugs em C, C++, Rust ou MicroPython. Aciona em: especificar firmware, planejar recurso embarcado, projetar driver, tarefa de microcontrolador, validar hardware, teste HIL, quick fix embarcado, requisitos de tempo real ou segurança.'
argument-hint: '[objetivo, bug ou recurso do firmware]'
user-invocable: true
---

# SDD para software embarcado

Use esta skill para conduzir desenvolvimento orientado a especificações em microcontroladores. A especificação é a fonte de intenção e deve permanecer ancorada ao recurso depois da implementação; o código continua sendo revisável e nunca deve ser tratado como artefato descartável sem validação humana.

## Princípios

1. **Dimensione o processo pelo risco e pela complexidade.** Não imponha uma cadeia longa a um bug local.
2. **Especifique comportamento observável.** Inclua estados, eventos, entradas, saídas, erros, limites, temporização e recuperação.
3. **Não invente detalhes de hardware.** Confirme MCU, placa, clock, periféricos, pinos, níveis elétricos, toolchain, RTOS e versão do SDK; marque lacunas como `A CONFIRMAR`.
4. **Separe intenção de decisão técnica, mas registre o vínculo.** Requisitos dizem o que deve ocorrer; design registra como a plataforma satisfaz o requisito e quais restrições não podem ser violadas.
5. **Trate recursos finitos como requisitos.** Memória RAM/flash, CPU, stack, energia, latência, jitter, largura de banda e ciclos de escrita devem ter limites verificáveis.
6. **Falha é comportamento.** Defina watchdog, brownout, timeout, perda de comunicação, valores inválidos, overflow, reset e estado seguro quando forem relevantes.
7. **Toda tarefa termina com evidência.** Teste unitário, análise estática, build, simulação, teste de bancada ou HIL devem ser escolhidos de acordo com o risco.
8. **Nunca declare hardware validado por teste apenas em host.** Diferencie claramente `HOST`, `SIMULADOR`, `BANCADA` e `HIL`.
9. **Pequenas iterações preservam controle.** Atualize a especificação quando o comportamento real ou uma decisão aprovada mudar.
10. **Declare contratos de concorrência antes de implementar.** Identifique toda variável compartilhada entre ISR, DMA e contexto de tarefa/loop principal. Especifique `volatile`, operações atômicas, seções críticas, barreiras de memória e ownership de periférico; nunca assuma que uma leitura-modificação-escrita de registrador de hardware é segura sem verificar acesso concorrente.

## Artefatos

Todo projeto mantém, na raiz do workspace, os arquivos de continuidade entre agentes:

- `README.md` — obrigatório. Apresentação e orientação geral: objetivo, descrição da aplicação, tecnologias, requisitos, como executar, estrutura de diretórios e o que um novo desenvolvedor precisa para iniciar. Conteúdo relativamente estável.
- `AGENTS.md` — obrigatório. Regras permanentes do projeto: regras de desenvolvimento, arquitetura e padrões obrigatórios, convenções de código, tecnologias e versões, comandos importantes, restrições, procedimentos de teste e critérios para alteração de arquivos. Todo agente deve ler `AGENTS.md` antes de modificar o código.
- `STATUS.md` — obrigatório. Estado atual do desenvolvimento: concluído, em andamento, pendente, problemas e erros conhecidos, testes realizados e pendentes, última alteração relevante e próximo passo recomendado. Atualizado a cada alteração significativa.
- `HANDOFF.md` — obrigatório. Transferência de trabalho entre agentes: contexto, estado atual, alterações realizadas, decisões, problemas, testes, pendências, próximo passo, cuidados e critério de conclusão. Atualizado ao final de uma sessão significativa.

Quando o projeto justificar, adicione:

- `SPECIFICATION.md` — especificação funcional e técnica do sistema, independente de implementação.
- `TASKS.md` — lista de tarefas do projeto com estado identificável (`[ ]` pendente, `[-]` em andamento, `[x]` concluída, `[!]` bloqueada).

O detalhamento técnico do SDD vive em `.specs/`:

```text
.specs/
├── project/
│   ├── constitution.md
│   └── ROADMAP.md
├── codebase/
│   ├── STACK.md
│   ├── TARGET.md
│   ├── ARCHITECTURE.md
│   ├── CONVENTIONS.md
│   ├── TESTING.md
│   ├── INTEGRATIONS.md
│   └── CONCERNS.md
├── features/<recurso>/
│   ├── spec.md
│   ├── context.md       # somente se houver decisões ambíguas
│   ├── design.md        # somente para mudanças grandes/complexas
│   └── tasks.md         # somente para mudanças grandes/complexas
└── quick/NNN-slug/
    ├── TASK.md
    └── SUMMARY.md
```

Use `./references/constitution.md` como ponto de partida para `AGENTS.md` (regras permanentes) e para `.specs/project/constitution.md`. Não copie a constituição para cada feature.

**Anti-bloat:** crie apenas os arquivos com conteúdo real; nunca crie placeholders vazios. Em projetos pequenos ou de um único firmware, consolide os documentos de `codebase/` em um único `CODEBASE.md` e registre isso em `STATUS.md`. Eleve a topologia completa somente quando o projeto crescer.

Todo recurso concluído deve deixar quatro resultados verificáveis, mesmo quando forem curtos: código compilável, testes executados, documentação atualizada e um registro de entrega. Em projetos existentes, mantenha `README.md` como porta de entrada para configurar, compilar, testar e executar o firmware; não crie uma segunda documentação concorrente. Inclua no `README.md` uma seção de licença; salvo decisão registrada em contrário, use a **Apache License 2.0** como padrão, contendo `Copyright <ano> <autor>` e o aviso oficial da licença com o link `http://www.apache.org/licenses/LICENSE-2.0`.

## Dimensionamento adaptativo

Antes de criar documentos, classifique a mudança:

| Escopo | Sinais | Artefatos e fluxo |
|---|---|---|
| **Pequeno** | Até 3 arquivos, um comportamento local, sem nova interface elétrica ou concorrência | `quick/TASK.md` → implementar → verificar → `SUMMARY.md` |
| **Médio** | Recurso claro, até 10 tarefas, altera um módulo, protocolo ou periférico conhecido | `spec.md` breve → design inline → implementar → verificar |
| **Grande** | Vários módulos, ISR/RTOS, protocolo, persistência, consumo, boot ou integração de hardware | `spec.md` com IDs → `design.md` → `tasks.md` → executar por tarefa |
| **Complexo** | Requisitos incertos, risco de segurança, timing crítico, nova placa/MCU, falha difícil de reproduzir | especificar → discutir lacunas → pesquisar → projetar → tarefas → implementar → validação incremental/HIL |

Eleve o nível se qualquer mudança aparentemente pequena afetar segurança, limites elétricos, dados persistentes, atualização de firmware, sincronização concorrente, ISR, DMA, clock tree, linker script ou comportamento após reset.

**Regra de segurança:** se uma lista de tarefas implícita passar de cinco itens ou revelar dependências, pare e crie `design.md` e `tasks.md` antes de continuar.

## Procedimento

### 1. Reconhecer o contexto

Em trabalho contínuo, comece por `AGENTS.md`, `STATUS.md` e `HANDOFF.md` para retomar regras, decisões, bloqueios e próximos passos sem refazer trabalho já validado. Em código existente, leia somente o necessário para localizar o caminho controlador e consulte, conforme o caso:

- `README.md`, `AGENTS.md`, `STATUS.md` e `.specs/project/constitution.md`;
- `TARGET.md`: família e revisão de silício, placa, tensão de alimentação; clock tree (fontes, PLL, divisores, frequências dos periféricos); mapa de pinos e funções alternativas; tabela de interrupções com prioridades e níveis de aninhamento; canais DMA com ownership e regiões de memória acessíveis; regiões de memória (ROM, RAM, DTCM/ITCM, CCM) e tamanhos configurados no linker script; instâncias de periféricos utilizadas; toolchain, SDK, RTOS e versões; modo de boot, fuse bits e opções de gravação relevantes;
- `TESTING.md` e `CONCERNS.md`;
- driver, HAL/BSP, task/thread, ISR, máquina de estados e testes vizinhos.

Registre fatos observados separadamente de hipóteses. Não substitua componentes ou protocolos existentes sem evidência.

### 2. Especificar o comportamento

Para especificar o sistema como um todo, use `SPECIFICATION.md` (independente de implementação). Para uma feature, crie `spec.md` ou `TASK.md` com:

- objetivo e fora de escopo;
- atores, estados e eventos;
- requisitos numerados `FR-###` para comportamento funcional;
- requisitos `NFR-###` para latência (incluindo WCET e latência de interrupção quando houver caminho de tempo real), frequência, jitter, memória (flash, RAM e stack), energia, disponibilidade, segurança, integridade de persistência e diagnóstico;
- critérios de aceitação no formato `DADO / QUANDO / ENTÃO`, incluindo casos de erro;
- matriz de rastreabilidade requisito → teste → evidência;
- premissas, riscos e perguntas bloqueadoras.

Cada requisito deve ser observável e testável. Evite frases como “rápido”, “robusto” ou “baixo consumo” sem métrica, condição e método de medição.

Para interfaces de hardware, registre unidade, escala, faixa, resolução, polaridade, debounce, tolerância, tempo de estabilização e comportamento fora da faixa. Para comunicação, registre framing, endianess, baud rate, timeout, CRC, versionamento, retries, perda de conexão e compatibilidade. Para ISR e DMA, registre vetor, prioridade, contexto de execução, tempo máximo de serviço, variáveis compartilhadas e qualificadores necessários.

Use uma tabela de interface para cada periférico significativo:

| Campo | Valor |
|---|---|
| Periférico / instância | ex.: `I2C1`, `ADC3_CH7`, `TIM2_CH1` |
| Pino(s) e função | ex.: `PB8 (SCL)`, `PB9 (SDA)` |
| Protocolo / modo | ex.: I2C 400 kHz Fast-mode, pullup externo 4,7 kΩ |
| Faixa / resolução | ex.: 0–3,3 V, 12 bits, Vref = 3,3 V |
| Taxa / período / temporização | ex.: 1 leitura/s ± 150 ms |
| ISR / DMA | ex.: `DMA2_Stream0_IRQn`, prioridade 5, buffer 64 bytes |
| Falha / fora da faixa | ex.: timeout > 100 ms → usar último valor válido, sinalizar flag de erro |

### 3. Discutir ambiguidades

Pergunte ao usuário somente decisões que mudem comportamento, risco, custo ou arquitetura, e agrupe todas as perguntas bloqueadoras em uma única interação em vez de perguntar uma a uma. Exemplos:

- estado seguro após falha ou reset;
- prioridade entre latência, consumo e precisão;
- política de timeout e retry;
- persistência e desgaste de flash/EEPROM;
- origem da verdade para calibração e parâmetros;
- tolerância a dados ausentes ou sensores desconectados;
- exigência de teste físico, simulador ou HIL;
- ownership exclusivo de periféricos e política de acesso concorrente;
- ownership e tempo de vida de buffers DMA (quem aloca, quem libera, quando são válidos);
- política de aninhamento de interrupções e prioridades máximas;
- sequência de inicialização e dependências entre periféricos.

Registre respostas em `context.md` e reflita-as na especificação. Se a resposta não vier, deixe a decisão como bloqueio explícito; não fabrique uma escolha.

### 4. Projetar quando necessário

Em `design.md`, mantenha o design proporcional e registre:

- módulos reutilizados e novos pontos de integração;
- costuras de teste: lógica pura (FSM, parsers, cálculos) separada de registradores/HAL para teste em host, com injeção de dependências ou stubs de hardware;
- fluxo de dados e máquina de estados;
- ownership de buffers, filas, locks e acesso a periféricos;
- contexto de execução: loop principal, ISR, task/thread e prioridades;
- orçamento de tempo e memória, stack/heap e estratégia para evitar fragmentação;
- tratamento de reset, watchdog, brownout e recuperação;
- interfaces HAL/BSP e dependências do SDK/RTOS;
- impacto em energia, EMI, atualização e compatibilidade;
- variáveis compartilhadas com ISR ou DMA: qualificador `volatile`, operações atômicas, seção crítica e barreiras de memória;
- registradores de hardware: nunca leia-modifique-escreva sem verificar acesso concorrente e bits de status relevantes;
- ADRs inline para decisões de impacto arquitetural: `ADR-###: título | contexto | decisão | consequências`;
- alternativas rejeitadas e motivo.

Qualquer decisão que não seja puramente funcional deve estar neste documento ou em `STATUS.md`, nunca escondida em uma tarefa vaga.

### 5. Quebrar em tarefas

Para mudanças grandes/complexas, registre a lista de tarefas em `TASKS.md` (estados `[ ]`, `[-]`, `[x]`, `[!]`) e detalhe cada tarefa com:

```text
Tarefa: T-### <verbo + resultado>
Requisitos: FR-###, NFR-###
Onde: arquivos, símbolos, periféricos ou interfaces
Depende de: T-### ou nenhum
Reutiliza: implementação/teste existente
Feito quando: comportamento e limites verificáveis
Testes: tipo, nível e cenário
Gate: comando ou evidência obrigatória
```

Prefira tarefas pequenas e ordenadas por risco: primeiro contrato e teste, depois implementação, integração e validação física. Tarefas paralelas só podem tocar superfícies sem dependência compartilhada.

Cada `tasks.md` deve terminar com uma seção `Entregáveis e aceite` contendo:

- arquivos de código, testes e documentação esperados;
- comando(s) de compilação para o alvo e configuração utilizada;
- comando(s) de teste e nível de evidência (`HOST`, `SIMULADOR`, `BANCADA` ou `HIL`);
- critérios de aceite rastreados por ID, incluindo limites de timing, memória, energia e comunicação quando aplicáveis;
- pendências, riscos residuais e responsável pela validação física.

### 6. Executar

Antes de editar, liste as etapas atômicas. Em cada tarefa:

1. leia `AGENTS.md`, o artefato correspondente e a constituição;
2. reutilize padrões locais e confirme o símbolo controlador;
3. altere o menor conjunto de arquivos;
4. execute o gate imediatamente;
5. atualize rastreabilidade e marque desvios da especificação como `SPEC_DEVIATION`;
6. atualize `STATUS.md` com decisões, bloqueios ou lições relevantes;
7. mantenha o `README.md` correto para qualquer mudança em instalação, configuração, compilação, testes, gravação ou execução;
8. gere ou atualize `HANDOFF.md` para o próximo agente quando houver trabalho incompleto, validação pendente ou contexto que não seja óbvio no código.

Não faça commit automaticamente. Se o usuário pedir commits, mantenha commits atômicos por tarefa e não misture refatoração não relacionada.

O `HANDOFF.md` deve ser objetivo e seguir o protocolo de continuidade: contexto, estado atual, alterações realizadas, decisões, problemas, testes, pendências, próximo passo, cuidados e critério de conclusão. Se não houver continuidade prevista, registre essa decisão em `SUMMARY.md` em vez de criar um handoff vazio.

### 7. Verificar

Escolha a evidência mínima suficiente e aumente-a conforme o risco:

- **Host:** testes unitários de lógica pura, parser, cálculo e máquina de estados;
- **Build:** compilação para o alvo, warnings tratados conforme a constituição, tamanho de flash/RAM e artefato gerado identificado;
- **Estático:** formatter, linter, análise estática (`cppcheck`, `clang-tidy`), checagem de concorrência; quando aplicável, conformidade MISRA-C/MISRA-C++ e análise de uso de stack pelo mapa de linker (`-fstack-usage`) ou ferramenta dedicada;
- **Simulador:** periféricos e temporização modelados;
- **Bancada:** pinos, níveis, sensores, atuadores, reset, watchdog e consumo;
- **HIL:** integração com hardware real, timing, comunicação e cenários de falha.

A validação deve registrar comando, ambiente, versão de toolchain/SDK, resultado, artefato de compilação e limitações. O gate mínimo de implementação é: compilar o código afetado, executar os testes disponíveis e confirmar que os critérios de aceite relevantes foram verificados. Não esconda testes impossíveis de executar por falta de hardware: marque-os como pendentes e explique o risco residual.

Para cada critério de aceite, registre `PASS`, `FAIL` ou `PENDENTE`, com a evidência correspondente. Um build bem-sucedido não substitui testes comportamentais; testes em host não substituem compilação para o alvo; e compilação para o alvo não substitui validação elétrica ou temporal em hardware quando essa validação for exigida.

### 8. Ancorar e encerrar

Antes de finalizar, confirme:

- todos os requisitos têm implementação ou justificativa explícita;
- todos os critérios de aceite têm teste/evidência ou pendência registrada;
- documentação e código não divergem;
- `README.md` explica como configurar, compilar, testar e executar o estado atual do projeto. Sempre que possível inclua instruções de gravação, monitoramento, validação física e diagramas mermaid para ilustrar fluxo, máquina de estados e arquitetura;
- `README.md` declara a licença do projeto (padrão Apache License 2.0, com `Copyright <ano> <autor>` e o link oficial `http://www.apache.org/licenses/LICENSE-2.0`, salvo decisão registrada em contrário);
- `.gitignore` e `.gitattributes` estão corretos para o alvo e toolchain;
- o código afetado compilou para o alvo, ou a impossibilidade está registrada em `HANDOFF.md`;
- limites de tempo, memória, energia e comunicação foram verificados quando aplicáveis;
- comportamento de erro, reset e recuperação foi considerado;
- `AGENTS.md` e `STATUS.md` estão atualizados e consistentes com o código;
- `TASKS.md` reflete o estado das tarefas, quando usado;
- `SUMMARY.md` registra entregáveis, arquivos, compilação, testes, critérios de aceite, desvios e riscos residuais;
- `HANDOFF.md` registra claramente qualquer continuidade necessária para o próximo agente;
- a especificação continua útil para a próxima manutenção do recurso.

## Formato rápido

### Bug ou correção local

Para um bug pequeno, escreva apenas:

```markdown
# TASK: <descrição>

## Comportamento atual
## Comportamento esperado
## Hipótese falsificável
## Escopo e riscos embarcados
- alvo/periférico afetado:
- timing/concorrência:
- falha/reset:
- memória/energia:

## Critérios de aceitação
- [ ] CA-001: DADO ... QUANDO ... ENTÃO ...
- [ ] CA-002: ... (inclua casos de erro, falha e reset)

## Plano atômico
1. ...

## Verificação
- Gate:
- Compilação do alvo:
- Testes executados:
- Nível: HOST | SIMULADOR | BANCADA | HIL
- Evidência esperada:

## Entregáveis
- [ ] Código implementado
- [ ] Testes criados ou atualizados
- [ ] `README.md` atualizado, quando instalação, compilação, teste ou execução mudarem
- [ ] `AGENTS.md` e `STATUS.md` atualizados, quando houver alteração significativa
- [ ] `TASKS.md` atualizado, quando usado
- [ ] `SUMMARY.md` com resultados
- [ ] `HANDOFF.md` criado ou atualizado se houver continuidade
```

### Novo driver ou periférico

```markdown
# TASK: <driver: nome do periférico ou interface>

## Objetivo e periférico
- alvo/instância:
- pinos e funções alternativas:
- protocolo, modo e temporização:

## Interface pública (API)
- funções, callbacks ou tipos principais

## Comportamento esperado
- estados e transições:
- eventos disparadores:
- saídas e efeitos observáveis:

## Escopo e restrições embarcadas
- ISR/DMA (vetor, prioridade, contexto de execução):
- volatile/atômico/seção crítica:
- memória (RAM, stack, buffers DMA):
- energia/clock gate:
- comportamento na falha de hardware:

## Critérios de aceitação
- [ ] CA-001: DADO ... QUANDO ... ENTÃO ...
- [ ] CA-002: ... (inclua casos de erro, falha e reset)

## Plano atômico
1. ...

## Verificação
- Gate:
- Compilação do alvo:
- Testes executados:
- Nível: HOST | SIMULADOR | BANCADA | HIL
- Evidência esperada:

## Entregáveis
- [ ] Driver implementado (HAL separado de lógica de aplicação)
- [ ] Testes unitários do driver
- [ ] Tabela de interface preenchida em `spec.md`
- [ ] `README.md` atualizado se necessário
- [ ] `AGENTS.md` e `STATUS.md` atualizados, quando houver alteração significativa
- [ ] `SUMMARY.md` com resultados
```

## Saída esperada da skill

Ao concluir, apresente uma síntese curta com: escopo escolhido, entregáveis criados/atualizados, requisitos e critérios de aceite atendidos, comando de compilação e resultado, testes e níveis executados, documentação atualizada (`README.md`, `AGENTS.md`, `STATUS.md` e `TASKS.md` quando usados), `HANDOFF.md` produzido quando aplicável, desvios da especificação e riscos ou validações de hardware ainda pendentes.
