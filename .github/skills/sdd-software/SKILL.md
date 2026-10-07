---
name: sdd-software
description: 'Planejar e implementar software de propósito geral (web, backend, APIs, serviços, apps) com Spec-Driven Development adaptativo. Use para especificar funcionalidades, endpoints, integrações, regras de negócio, mudanças de schema de dados, correções de bugs e refatorações em qualquer linguagem ou framework. Aciona em: especificar funcionalidade, planejar recurso de software, projetar API, criar endpoint, corrigir bug, quick fix, requisitos não funcionais, spec-driven development, SDD.'
argument-hint: '[objetivo, bug ou funcionalidade de software]'
user-invocable: true
---

# SDD para desenvolvimento de software

Use esta skill para conduzir desenvolvimento orientado a especificações em software de propósito geral (web, backend, APIs, serviços, aplicações desktop/mobile). A especificação é a fonte de intenção e deve permanecer ancorada à funcionalidade depois da implementação; o código continua sendo revisável e nunca deve ser tratado como artefato descartável sem validação humana.

Esta skill não cobre firmware ou software embarcado em microcontroladores (ISR, DMA, RTOS, HIL, restrições elétricas). Para isso, use a skill `sdd-embarcado`.

## Princípios

1. **Dimensione o processo pelo risco e pela complexidade.** Não imponha uma cadeia longa a um bug local.
2. **Especifique comportamento observável.** Inclua estados, eventos, entradas, saídas, erros, limites e casos de borda.
3. **Não invente detalhes técnicos.** Confirme linguagem, framework, runtime, versão, banco de dados, infraestrutura e serviços externos; marque lacunas como `A CONFIRMAR`.
4. **Separe intenção de decisão técnica, mas registre o vínculo.** Requisitos dizem o que deve ocorrer; design registra como a solução satisfaz o requisito e quais restrições não podem ser violadas.
5. **Trate recursos finitos e metas de qualidade como requisitos.** Latência, throughput, disponibilidade, custo, escalabilidade e limites de uso (rate limit, quota) devem ter valores verificáveis.
6. **Falha é comportamento.** Defina erro, timeout, retry, fallback, estado inconsistente e mensagens ao usuário quando forem relevantes.
7. **Toda tarefa termina com evidência.** Teste unitário, integração, contrato/E2E, análise estática ou validação manual devem ser escolhidos de acordo com o risco.
8. **Nunca declare produção validada por teste apenas local.** Diferencie claramente `LOCAL`, `CI`, `STAGING` e `PRODUÇÃO`.
9. **Pequenas iterações preservam controle.** Atualize a especificação quando o comportamento real ou uma decisão aprovada mudar.
10. **Declare contratos de concorrência e consistência antes de implementar.** Identifique estado compartilhado entre threads, processos, filas ou requisições concorrentes. Especifique transações, locks otimistas/pessimistas, idempotência e ordenação; nunca assuma que uma operação de leitura-modificação-escrita é segura sem verificar acesso concorrente.
11. **Documentação é entregável, não um afterthought.** README, AGENTS.md, STATUS.md, TASKS.md e documentação técnica relevante devem refletir o comportamento real do sistema ao final de cada tarefa; documentação desatualizada é tratada como defeito, não como pendência menor.

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

**Anti-bloat:** crie apenas os arquivos com conteúdo real; nunca crie placeholders vazios. Em projetos pequenos ou monolitos simples, consolide os documentos de `codebase/` em um único `CODEBASE.md` e registre isso em `STATUS.md`. Eleve a topologia completa somente quando o projeto crescer.

Todo recurso concluído deve deixar quatro resultados verificáveis, mesmo quando forem curtos: código funcional, testes executados, documentação atualizada e um registro de entrega. Em projetos existentes, mantenha `README.md` como porta de entrada para instalar dependências, configurar, compilar/buildar, testar e executar a aplicação; não crie uma segunda documentação concorrente.

Todo projeto deve ter, na raiz, um arquivo `LICENSE` e um arquivo `.gitignore`; se não existirem, crie-os no início do trabalho, não espere o encerramento da tarefa. Salvo decisão registrada em contrário, use a **Apache License 2.0** como padrão para `LICENSE`, contendo `Copyright <ano> <autor>` e o aviso oficial da licença com o link `http://www.apache.org/licenses/LICENSE-2.0`; mantenha a seção de licença do `README.md` consistente com o arquivo `LICENSE`. O `.gitignore` deve cobrir artefatos de build, dependências, arquivos de ambiente/segredos e saídas de IDE compatíveis com a stack do projeto.

## Dimensionamento adaptativo

Antes de criar documentos, classifique a mudança:

| Escopo | Sinais | Artefatos e fluxo |
|---|---|---|
| **Pequeno** | Até 3 arquivos, um comportamento local, sem novo contrato de API ou schema de dados | `quick/TASK.md` → implementar → verificar → `SUMMARY.md` |
| **Médio** | Recurso claro, até 10 tarefas, altera um módulo, endpoint ou serviço conhecido | `spec.md` breve → design inline → implementar → verificar |
| **Grande** | Vários módulos/serviços, novo contrato de API, mudança de schema de dados, integração externa nova, concorrência relevante | `spec.md` com IDs → `design.md` → `tasks.md` → executar por tarefa |
| **Complexo** | Requisitos incertos, risco de segurança, migração de dados, mudança incompatível (breaking change), performance crítica, nova arquitetura | especificar → discutir lacunas → pesquisar → projetar → tarefas → implementar → validação incremental |

Eleve o nível se qualquer mudança aparentemente pequena afetar autenticação/autorização, dados sensíveis (PII, pagamento, credenciais), schema de dados persistente, contrato público de API, estado compartilhado concorrente ou compatibilidade com clientes existentes.

**Regra de segurança:** se uma lista de tarefas implícita passar de cinco itens ou revelar dependências, pare e crie `design.md` e `tasks.md` antes de continuar.

## Procedimento

### 1. Reconhecer o contexto

Em trabalho contínuo, comece por `AGENTS.md`, `STATUS.md` e `HANDOFF.md` para retomar regras, decisões, bloqueios e próximos passos sem refazer trabalho já validado. Em código existente, leia somente o necessário para localizar o caminho controlador e consulte, conforme o caso:

- `README.md`, `AGENTS.md`, `STATUS.md` e `.specs/project/constitution.md`;
- `STACK.md`: linguagem, framework, runtime e versões; gerenciador de pacotes e build; banco de dados e ORM/driver; infraestrutura de deploy e variáveis de ambiente;
- `ARCHITECTURE.md`, `CONVENTIONS.md`, `TESTING.md` e `INTEGRATIONS.md` (serviços externos, filas, webhooks);
- `CONCERNS.md`;
- módulo, serviço, controlador, rota, schema e testes vizinhos.

Registre fatos observados separadamente de hipóteses. Não substitua bibliotecas ou padrões existentes sem evidência.

### 2. Especificar o comportamento

Para especificar o sistema como um todo, use `SPECIFICATION.md` (independente de implementação). Para uma feature, crie `spec.md` ou `TASK.md` com:

- objetivo e fora de escopo;
- atores, estados e eventos;
- requisitos numerados `FR-###` para comportamento funcional;
- requisitos `NFR-###` para latência, throughput, disponibilidade, escalabilidade, segurança, custo e observabilidade;
- critérios de aceitação no formato `DADO / QUANDO / ENTÃO`, incluindo casos de erro;
- matriz de rastreabilidade requisito → teste → evidência;
- premissas, riscos e perguntas bloqueadoras.

Cada requisito deve ser observável e testável. Evite frases como "rápido", "robusto" ou "escalável" sem métrica, condição e método de medição.

Para APIs e contratos de dados, registre uma tabela de interface para cada endpoint ou operação significativa:

| Campo | Valor |
|---|---|
| Endpoint / operação | ex.: `POST /orders` |
| Autenticação / autorização | ex.: Bearer JWT, role `admin` |
| Payload / schema | ex.: JSON schema, campos obrigatórios e opcionais |
| Respostas | ex.: `200`, `4xx`, `5xx` e formato do corpo |
| Idempotência / concorrência | ex.: chave de idempotência, lock otimista, versão do registro |
| Limites | ex.: rate limit, tamanho máximo de payload, paginação |
| Falha / timeout | ex.: retry com backoff, circuit breaker, fallback |

### 3. Discutir ambiguidades

Pergunte ao usuário somente decisões que mudem comportamento, risco, custo ou arquitetura, e agrupe todas as perguntas bloqueadoras em uma única interação em vez de perguntar uma a uma. Exemplos:

- fonte da verdade para os dados e ownership entre serviços;
- política de retry, timeout e circuit breaker;
- requisitos de segurança: autenticação, autorização, dados sensíveis/PII, criptografia em trânsito e em repouso;
- compatibilidade retroativa e versionamento de API/contrato;
- estratégia de migração de dados e rollback;
- meta de performance/SLA e volume esperado;
- ambiente de deploy, feature flag e estratégia de rollout;
- política de idempotência para operações repetíveis;
- tolerância a dados ausentes, duplicados ou fora de ordem.

Registre respostas em `context.md` e reflita-as na especificação. Se a resposta não vier, deixe a decisão como bloqueio explícito; não fabrique uma escolha.

### 4. Projetar quando necessário

Em `design.md`, mantenha o design proporcional e registre:

- módulos reutilizados e novos pontos de integração;
- costuras de teste: lógica de negócio pura separada de I/O (banco de dados, rede, sistema de arquivos), com injeção de dependências, mocks ou stubs;
- fluxo de dados e máquina de estados;
- contratos de API, schema de dados e migrações;
- ownership de transações, estado compartilhado e cache;
- modelo de concorrência: threads, processos, filas, workers assíncronos, locks e ordenação de eventos;
- tratamento de erro e resiliência: retry, timeout, circuit breaker, fallback e mensagens ao usuário;
- segurança: validação de entrada, autenticação/autorização, gestão de segredos e superfícies de ataque (injeção, XSS, CSRF, deserialização);
- observabilidade: logs estruturados, métricas e tracing;
- impacto em performance, custo e compatibilidade;
- ADRs inline para decisões de impacto arquitetural: `ADR-###: título | contexto | decisão | consequências`;
- alternativas rejeitadas e motivo.

Qualquer decisão que não seja puramente funcional deve estar neste documento ou em `STATUS.md`, nunca escondida em uma tarefa vaga.

### 5. Quebrar em tarefas

Para mudanças grandes/complexas, registre a lista de tarefas em `TASKS.md` (estados `[ ]`, `[-]`, `[x]`, `[!]`) e detalhe cada tarefa com:

```text
Tarefa: T-### <verbo + resultado>
Requisitos: FR-###, NFR-###
Onde: arquivos, módulos, endpoints ou serviços
Depende de: T-### ou nenhum
Reutiliza: implementação/teste existente
Feito quando: comportamento e limites verificáveis
Testes: tipo, nível e cenário
Gate: comando ou evidência obrigatória
```

Prefira tarefas pequenas e ordenadas por risco: primeiro contrato e teste, depois implementação, integração e validação. Tarefas paralelas só podem tocar superfícies sem dependência compartilhada.

Cada `tasks.md` deve terminar com uma seção `Entregáveis e aceite` contendo:

- arquivos de código, testes e documentação esperados;
- comando(s) de build/compilação e de execução;
- comando(s) de teste e nível de evidência (`LOCAL`, `CI`, `STAGING` ou `PRODUÇÃO`);
- critérios de aceite rastreados por ID, incluindo limites de latência, throughput e segurança quando aplicáveis;
- pendências, riscos residuais e responsável pela validação.

### 6. Executar

Antes de editar, liste as etapas atômicas. Em cada tarefa:

1. leia `AGENTS.md`, o artefato correspondente e a constituição;
2. reutilize padrões locais e confirme o símbolo controlador;
3. altere o menor conjunto de arquivos;
4. execute o gate imediatamente;
5. atualize rastreabilidade e marque desvios da especificação como `SPEC_DEVIATION`;
6. atualize `STATUS.md` com decisões, bloqueios ou lições relevantes;
7. mantenha toda a documentação afetada atualizada (`README.md`, documentação técnica, comentários de API) para qualquer mudança em instalação, configuração, build, testes, execução ou comportamento observável;
8. gere ou atualize `HANDOFF.md` para o próximo agente quando houver trabalho incompleto, validação pendente ou contexto que não seja óbvio no código.

Não faça commit automaticamente. Se o usuário pedir commits, mantenha commits atômicos por tarefa e não misture refatoração não relacionada.

O `HANDOFF.md` deve ser objetivo e seguir o protocolo de continuidade: contexto, estado atual, alterações realizadas, decisões, problemas, testes, pendências, próximo passo, cuidados e critério de conclusão. Se não houver continuidade prevista, registre essa decisão em `SUMMARY.md` em vez de criar um handoff vazio.

### 7. Verificar

Escolha a evidência mínima suficiente e aumente-a conforme o risco:

- **Estático:** formatter, linter, checagem de tipos, análise de segurança de dependências;
- **Unitário:** lógica de negócio, parsers, cálculos e regras isoladas de I/O;
- **Integração:** banco de dados, filas, cache e serviços externos reais ou em contêiner;
- **Contrato/E2E:** fluxo completo da API ou da interface, incluindo autenticação e casos de erro;
- **Manual/exploratório:** quando não houver automação viável, registre o roteiro executado;
- **Produção:** observabilidade, canary/rollout gradual e monitoramento pós-deploy, quando aplicável.

A validação deve registrar comando, ambiente, versão de runtime/dependências, resultado e limitações. O gate mínimo de implementação é: build/compilação bem-sucedida do código afetado, execução dos testes disponíveis e confirmação de que os critérios de aceite relevantes foram verificados. Não esconda testes impossíveis de executar por falta de ambiente: marque-os como pendentes e explique o risco residual.

Para cada critério de aceite, registre `PASS`, `FAIL` ou `PENDENTE`, com a evidência correspondente. Um build bem-sucedido não substitui testes comportamentais; testes locais não substituem validação em CI/staging quando essa validação for exigida.

### 8. Ancorar e encerrar

Antes de finalizar, confirme:

- todos os requisitos têm implementação ou justificativa explícita;
- todos os critérios de aceite têm teste/evidência ou pendência registrada;
- documentação e código não divergem;
- `README.md` explica como instalar, configurar, buildar, testar e executar o estado atual do projeto. Sempre que possível inclua diagramas mermaid para ilustrar fluxo, arquitetura e modelo de dados;
- `LICENSE` e `.gitignore` existem na raiz do projeto; se não existiam, foram criados nesta tarefa;
- `README.md` declara a licença do projeto e é consistente com o arquivo `LICENSE` (padrão Apache License 2.0, com `Copyright <ano> <autor>` e o link oficial `http://www.apache.org/licenses/LICENSE-2.0`, salvo decisão registrada em contrário);
- `.gitignore` está correto para a stack utilizada;
- o código afetado buildou/compilou, ou a impossibilidade está registrada em `HANDOFF.md`;
- limites de performance, segurança e compatibilidade foram verificados quando aplicáveis;
- comportamento de erro, retry e recuperação foi considerado;
- `AGENTS.md` e `STATUS.md` estão atualizados e consistentes com o código;
- `TASKS.md` reflete o estado das tarefas, quando usado;
- `SUMMARY.md` registra entregáveis, arquivos, build, testes, critérios de aceite, desvios e riscos residuais;
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
## Escopo e riscos
- módulo/serviço afetado:
- dados/segurança:
- compatibilidade:
- concorrência:

## Critérios de aceitação
- [ ] CA-001: DADO ... QUANDO ... ENTÃO ...
- [ ] CA-002: ... (inclua casos de erro)

## Plano atômico
1. ...

## Verificação
- Gate:
- Build:
- Testes executados:
- Nível: LOCAL | CI | STAGING | PRODUÇÃO
- Evidência esperada:

## Entregáveis
- [ ] Código implementado
- [ ] Testes criados ou atualizados
- [ ] `README.md` atualizado, quando instalação, build, teste ou execução mudarem
- [ ] `AGENTS.md` e `STATUS.md` atualizados, quando houver alteração significativa
- [ ] `TASKS.md` atualizado, quando usado
- [ ] `SUMMARY.md` com resultados
- [ ] `HANDOFF.md` criado ou atualizado se houver continuidade
```

### Nova funcionalidade ou endpoint

```markdown
# TASK: <funcionalidade ou endpoint>

## Objetivo
- módulo/serviço:
- consumidores (usuários, clientes, outros serviços):

## Interface pública (API/contrato)
- endpoint, operação, evento ou função principal
- payload/schema de entrada e saída

## Comportamento esperado
- estados e transições:
- eventos disparadores:
- saídas e efeitos observáveis:

## Escopo e restrições
- autenticação/autorização:
- concorrência/idempotência:
- dados persistentes/migração:
- performance/escalabilidade:
- comportamento na falha (retry, timeout, fallback):

## Critérios de aceitação
- [ ] CA-001: DADO ... QUANDO ... ENTÃO ...
- [ ] CA-002: ... (inclua casos de erro)

## Plano atômico
1. ...

## Verificação
- Gate:
- Build:
- Testes executados:
- Nível: LOCAL | CI | STAGING | PRODUÇÃO
- Evidência esperada:

## Entregáveis
- [ ] Funcionalidade implementada
- [ ] Testes unitários e de integração
- [ ] Tabela de interface preenchida em `spec.md`
- [ ] `README.md` atualizado se necessário
- [ ] `AGENTS.md` e `STATUS.md` atualizados, quando houver alteração significativa
- [ ] `SUMMARY.md` com resultados
```

## Saída esperada da skill

Ao concluir, apresente uma síntese curta com: escopo escolhido, entregáveis criados/atualizados, requisitos e critérios de aceite atendidos, comando de build/teste e resultado, testes e níveis executados, documentação atualizada (`README.md`, `AGENTS.md`, `STATUS.md` e `TASKS.md` quando usados), confirmação de que `LICENSE` e `.gitignore` existem, `HANDOFF.md` produzido quando aplicável, desvios da especificação e riscos ou validações ainda pendentes.
