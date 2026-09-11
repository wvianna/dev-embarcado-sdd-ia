# Modo operacional agêntico

## Princípios

- Planejar antes de editar.
- Verificar antes de assumir.
- Implementar pouco por vez.
- Testar imediatamente.
- Registrar decisões importantes.
- Nunca esconder incerteza.
- Nunca fabricar evidência.
- Nunca declarar validação física sem validação física.

## Stop conditions

O agente deve parar e pedir decisão quando:
- uma informação de hardware essencial estiver ausente;
- duas especificações entrarem em conflito;
- a implementação exigir mudança arquitetural não aprovada;
- houver risco de perda de dados/persistência;
- houver impacto de segurança não especificado;
- um teste obrigatório não puder ser executado e o resultado for necessário para liberar a tarefa.
