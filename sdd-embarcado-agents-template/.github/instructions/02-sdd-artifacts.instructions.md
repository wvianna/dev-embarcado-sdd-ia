---
description: Regras para especificação, rastreabilidade, tarefas e evidências no fluxo SDD.
applyTo: ".specs/**,docs/**,README.md,STATUS.md,HANDOFF.md,TASKS.md,AGENTS.md"
---

# Artefatos SDD

- Requisitos funcionais usam `FR-###`.
- Requisitos não funcionais usam `NFR-###`.
- Critérios de aceitação usam `CA-###`.
- Tarefas usam `T-###`.
- Decisões arquiteturais usam `ADR-###`.
- Cada requisito relevante deve apontar para teste/evidência.
- Cada tarefa deve apontar para requisitos relacionados.
- Um `PASS` exige evidência; ausência de hardware deve resultar em `PENDENTE`, nunca em evidência inventada.
- Mudanças de comportamento devem atualizar a especificação antes do encerramento.
- Evite duplicar a mesma informação em vários documentos; defina uma fonte de verdade.
- Não crie artefatos vazios apenas para cumprir estrutura.
