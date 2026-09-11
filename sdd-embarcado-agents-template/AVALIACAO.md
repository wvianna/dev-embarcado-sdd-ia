# Avaliação dos artefatos SDD + agentes

## Diagnóstico

Os anexos apresentam uma base forte para desenvolvimento embarcado orientado a especificações. Os pontos mais positivos são:

- preocupação explícita com hardware real, timing, memória, energia e segurança;
- distinção HOST/SIMULADOR/BANCADA/HIL;
- rastreabilidade por requisitos;
- uso de STATUS/HANDOFF;
- preocupação com ISR/DMA/concorrência;
- agentes especializados por etapa.

## Principais oportunidades de melhoria

### 1. O fluxo está linear demais

Os 12 agentes sugerem uma cadeia completa mesmo para alterações pequenas. Isso aumenta custo de contexto e latência.

**Melhoria:** adicionar um `task-router` e tornar o fluxo adaptativo ao risco.

### 2. Falta um gate independente de verificação

Test Engineer e Code Reviewer são importantes, mas não existe um agente explicitamente responsável por responder:

> "Há evidência suficiente para declarar esta tarefa concluída?"

**Melhoria:** `verification-gate.agent.md`.

### 3. Economia de contexto ainda não é uma regra operacional permanente

A skill recomenda leituras iniciais de vários artefatos. Isso pode ser excessivo em tarefas pequenas.

**Melhoria:** regras `.github/instructions/00-context-economy.instructions.md` e protocolo de contexto.

### 4. Falta separação explícita entre estado real e memória da conversa

Agentes podem assumir que um arquivo continua igual ao que foi visto anteriormente.

**Melhoria:** exigir `git status` + `git diff` antes de alterações.

### 5. Rastreabilidade está conceitualmente definida, mas não operacionalizada

A skill menciona requisitos, testes e evidências, e o Orchestrator menciona uma matriz, mas falta um artefato padronizado.

**Melhoria:** `TRACEABILITY.md` com FR/NFR → CA → Tarefa → Código → Teste → Evidência.

### 6. Handoff pode gerar duplicação

`STATUS.md`, `HANDOFF.md` e `SUMMARY.md` podem acabar contendo a mesma informação.

**Melhoria:** definir o papel de cada documento e usar `HANDOFF.md` somente quando houver continuidade.

### 7. Falta um mecanismo explícito de "stop"

Um agente agêntico precisa saber quando não deve continuar sozinho.

**Melhoria:** regras de parada para hardware indefinido, conflitos de especificação, mudanças arquiteturais, segurança, persistência e validação obrigatória ausente.

### 8. Definition of Done está espalhada

Há gates em vários documentos, mas falta um checklist único para encerramento.

**Melhoria:** `DEFINITION-OF-DONE.md`.

## Arquitetura recomendada

```text
Solicitação
   ↓
Task Router
   ↓
Classificação de risco/complexidade
   ├── Pergunta → resposta
   ├── Quick fix → implementar → verificar
   ├── Médio → especificar → implementar → testar → verificar
   └── Grande → requisitos → arquitetura → tarefas
                                      ↓
                              implementação
                                      ↓
                                  testes
                                      ↓
                                  revisão
                                      ↓
                            segurança (se aplicável)
                                      ↓
                              verification gate
                                      ↓
                         documentação / integração
                                      ↓
                                  release
```

## Recomendação sobre a Constituição

A Constituição deve conter apenas princípios estáveis e de alta prioridade.

Regras operacionais como:
- formato de handoff;
- economia de contexto;
- comandos de inspeção;
- comportamento específico de agentes;

devem ficar preferencialmente em Rules/Instructions ou nos próprios agentes.

Isso reduz o risco de transformar a Constituição em um documento excessivamente grande.

## Recomendação sobre os agentes existentes

Os agentes atuais estão bem separados por responsabilidade, mas precisam de contratos mais explícitos:

- entradas esperadas;
- artefatos que podem ler;
- artefatos que podem alterar;
- pré-condições;
- pós-condições;
- gate de saída;
- condição de parada.

Isso diminui sobreposição e torna o sistema mais previsível.

## Resultado

A estrutura original é boa como base. A principal evolução recomendada não é criar mais agentes indiscriminadamente, mas transformar o conjunto em um sistema **adaptativo, orientado a risco, com economia de contexto e gates objetivos**.

Os novos artefatos deste pacote implementam essa estratégia sem substituir os agentes especializados existentes.
