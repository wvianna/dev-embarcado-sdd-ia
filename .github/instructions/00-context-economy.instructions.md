---
description: Regras obrigatórias de economia de contexto e inspeção incremental do repositório.
applyTo: "**"
---

# Economia de contexto

1. Não faça varredura proativa do repositório.
2. Antes de ler arquivos, identifique a tarefa atual e os arquivos estritamente necessários.
3. Prefira `git status --short`, `git diff` e leituras direcionadas a comandos que despejem grandes volumes de conteúdo.
4. Antes de qualquer alteração de código, execute `git diff` e considere o resultado como o estado atual do workspace.
5. Nunca trate o histórico da conversa como fonte confiável do estado atual dos arquivos.
6. Não leia arquivos inteiros quando uma seção, símbolo ou intervalo for suficiente.
7. Não carregue arquivos não relacionados à tarefa.
8. Ao terminar uma tarefa, descarte mentalmente detalhes de arquivos e decisões que não sejam necessários para a continuidade.
9. Se a tarefa exigir contexto adicional, carregue-o incrementalmente e explique por que ele é necessário.
10. Não execute `find`, `tree`, `ls -R` ou equivalentes em grandes árvores sem necessidade explícita.

## Regra especial para alterações

Antes de editar código:
```bash
git status --short
git diff --stat
git diff
```

Se houver alterações pré-existentes, não as reverta nem as misture à tarefa atual.
