# Matriz de rastreabilidade

Use uma única matriz por projeto ou por feature grande.

| Requisito | Critério | Tarefa | Código | Teste | Evidência | Estado |
|---|---|---|---|---|---|---|
| FR-001 | CA-001 | T-001 | `src/...` | `test_...` | `build/...` | PASS |
| NFR-001 | CA-002 | T-002 | `src/...` | `test_...` | medição | PENDENTE |

## Regras

- Não marque `PASS` sem evidência.
- `PENDENTE` deve conter justificativa e risco residual.
- Uma alteração de requisito deve propagar-se para tarefa, teste e documentação.
