# Fluxo agêntico recomendado

## Princípio

O agente deve usar o menor fluxo capaz de produzir uma mudança segura, verificável e rastreável.

```mermaid
flowchart TD
    A[Solicitação] --> B[Task Router]
    B --> C{Complexidade e risco}
    C -->|Pergunta| D[Responder com evidência]
    C -->|Quick fix| E[TASK mínimo]
    C -->|Médio| F[Spec curta]
    C -->|Grande/complexo| G[Spec + Design + Tasks]
    E --> H[Implementer]
    F --> H
    G --> I[Architect]
    I --> J[Task Planner]
    J --> H
    H --> K[Test Engineer]
    K --> L[Code Reviewer]
    L --> M[Security Reviewer quando aplicável]
    M --> N[Verification Gate]
    N --> O[Documentation]
    O --> P[Integration]
    P --> Q[Release quando aplicável]
```

## Gates

### Gate 0 — Estado
`git status --short` + `git diff`.

### Gate 1 — Intenção
Requisito e critério de aceitação identificados.

### Gate 2 — Design
Necessário somente quando risco/complexidade exigir.

### Gate 3 — Implementação
Mudança mínima e rastreável.

### Gate 4 — Evidência
Build + testes + validações aplicáveis.

### Gate 5 — Encerramento
Documentação, rastreabilidade, estado e riscos atualizados.
