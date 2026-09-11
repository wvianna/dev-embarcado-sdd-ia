# Extensão agêntica para SDD embarcado

Este pacote complementa os perfis de agentes existentes com:

- regras de economia de contexto;
- regras específicas para firmware;
- controle de mudanças;
- roteamento de tarefas;
- gate formal de verificação;
- gerenciamento de handoff;
- matriz de rastreabilidade;
- Definition of Done;
- protocolo de contexto;
- fluxo agêntico adaptativo.

## Instalação

Copie:

```text
.github/agents/
.github/instructions/
docs/agentic/
```

para a raiz do projeto.

Depois integre as regras permanentes relevantes em `AGENTS.md` e `.specify/memory/constitution.md`.

## Estratégia

Não use todos os agentes em toda tarefa.

Bug local:
`task-router → implementer → verification-gate`

Feature média:
`task-router → requirements → implementer → test → review → verification`

Feature grande:
`planner → requirements → architect → task-planner → implementer → test → review → security → verification → documentation → integration`

Release:
`verification → security → documentation → integration → release`
