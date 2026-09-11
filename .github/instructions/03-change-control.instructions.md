---
description: Controle de mudanças, escopo e verificação antes de concluir uma tarefa.
applyTo: "**"
---

# Controle de mudanças

Antes de implementar:
1. Defina escopo e fora de escopo.
2. Identifique arquivos/componentes que realmente precisam mudar.
3. Execute `git diff`.
4. Defina o gate da tarefa.

Durante:
- Faça a menor alteração suficiente.
- Não refatore código não relacionado.
- Não altere testes apenas para fazê-los passar.
- Não introduza dependências sem justificar custo e benefício.
- Se descobrir necessidade de arquitetura diferente, pare e registre a decisão antes de prosseguir.

Depois:
1. Compile o alvo afetado.
2. Execute os testes disponíveis.
3. Execute análise estática/formatador quando aplicável.
4. Verifique critérios `CA-###`.
5. Atualize documentação e estado.
6. Relate limitações e riscos residuais.
