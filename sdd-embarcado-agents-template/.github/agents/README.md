# Perfis de Agents para VS Code

Conjunto de perfis Markdown para organizar um projeto de software com agentes especializados.

## Agents

1. `00-task-router.agent.md` — triagem e roteamento por complexidade e risco
2. `01-product-planner.agent.md` — planejamento, escopo e critérios de sucesso
3. `02-requirements-engineer.agent.md` — requisitos funcionais e não funcionais
4. `03-architect.agent.md` — arquitetura e decisões técnicas
5. `04-task-planner.agent.md` — decomposição em tarefas e backlog
6. `05-implementer.agent.md` — implementação
7. `06-code-reviewer.agent.md` — revisão de código
8. `07-test-engineer.agent.md` — testes automatizados e estratégia de testes
9. `08-security-reviewer.agent.md` — segurança (quando aplicável)
10. `09-documentation.agent.md` — documentação técnica e de usuário
11. `10-integration-agent.agent.md` — integração e validação entre componentes
12. `11-release-agent.agent.md` — preparação de release (quando aplicável)
13. `12-project-orchestrator.agent.md` — coordenação do fluxo completo (00–14)
14. `13-verification-gate.agent.md` — gate de verificação por evidências
15. `14-handoff-manager.agent.md` — continuidade entre sessões (quando aplicável)

## Estrutura recomendada

Coloque os arquivos de agentes em `.github/agents/`.

A especificação técnica vive em `.specs/`; os arquivos de continuidade ficam na raiz do projeto:

```text
.specs/    # especificação (project/, codebase/, features/, quick/)
TASKS.md   # backlog de tarefas
STATUS.md  # estado atual
HANDOFF.md # continuidade entre sessões
```

## Fluxo sugerido

Não use todos os agentes em toda tarefa:

- Bug local: `task-router → implementer → verification-gate`
- Feature média: `task-router → requirements-engineer → implementer → test-engineer → code-reviewer → verification-gate`
- Feature grande: `product-planner → requirements-engineer → architect → task-planner → implementer → test-engineer → code-reviewer → security-reviewer → verification-gate → documentation → integration-agent`
- Release: `verification-gate → security-reviewer → documentation → integration-agent → release-agent`

O `project-orchestrator` coordena o fluxo, mas não substitui os agentes especializados.
