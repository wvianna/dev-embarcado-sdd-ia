# Perfis de Agents para VS Code

Conjunto de perfis Markdown para organizar um projeto de software com agentes especializados.

## Agents

1. `01-product-planner.agent.md` — planejamento e escopo
2. `02-requirements-engineer.agent.md` — requisitos funcionais e não funcionais
3. `03-architect.agent.md` — arquitetura e decisões técnicas
4. `04-task-planner.agent.md` — decomposição em tarefas e backlog
5. `05-implementer.agent.md` — implementação
6. `06-code-reviewer.agent.md` — revisão de código
7. `07-test-engineer.agent.md` — testes automatizados e estratégia de testes
8. `08-security-reviewer.agent.md` — segurança
9. `09-documentation.agent.md` — documentação técnica e de usuário
10. `10-integration-agent.agent.md` — integração e validação entre componentes
11. `11-release-agent.agent.md` — preparação de release
12. `12-project-orchestrator.agent.md` — coordenação do fluxo completo

## Estrutura recomendada

Coloque os arquivos de agentes em:

`.github/agents/`

Os artefatos produzidos por eles podem ficar em:

```text
docs/
├── 01-planning/
├── 02-requirements/
├── 03-architecture/
├── 04-tasks/
├── 05-testing/
├── 06-security/
├── 07-documentation/
└── 08-release/
```

O código-fonte permanece nas pastas normais do projeto.

## Fluxo sugerido

`Planejar → Requisitos → Arquitetura → Tarefas → Implementar → Testar → Revisar → Documentar → Integrar → Release`

O `Project Orchestrator` pode coordenar esse fluxo, mas não deve substituir os agentes especializados.
