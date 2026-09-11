# Protocolo de contexto entre agentes

## Fonte de verdade

Prioridade recomendada:

1. estado real do workspace/Git;
2. Constituição;
3. especificação da feature;
4. design/ADR;
5. tasks;
6. código atual;
7. testes e evidências;
8. STATUS/HANDOFF;
9. histórico da conversa.

## Regra

O histórico da conversa pode fornecer intenção, mas nunca deve substituir a leitura do estado atual do workspace.

## Handoff

Um agente deve entregar ao próximo somente:
- o que foi feito;
- o que mudou;
- o que foi comprovado;
- o que falta;
- qual é o próximo passo.

Não transferir dumps de arquivos ou logs extensos sem necessidade.
