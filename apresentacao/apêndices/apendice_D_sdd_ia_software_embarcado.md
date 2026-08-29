# Apêndice A: Quality Gate de Firmware

O **Quality Gate** é o conjunto de verificações técnicas obrigatórias que devem ser executadas antes de marcar qualquer tarefa como concluída. Ele serve como uma barreira de proteção contra a falsa premissa de que "código compilado é código pronto", garantindo estabilidade no hardware real.

---

## Checklist de Validação (Gate)

```markdown
[ ] Requisitos observáveis
[ ] Alvo confirmado
[ ] Toolchain/SDK confirmados
[ ] Reset considerado
[ ] Watchdog considerado
[ ] Timing considerado
[ ] RAM/Flash considerados
[ ] Energia considerada
[ ] Testes executados no nível correto
[ ] Resultado registrado

```

---

## Detalhamento dos Critérios

### 1. Requisitos Observáveis

* O comportamento implementado gera um efeito mensurável externamente (logs via UART/RTT, alternância de GPIO, resposta de comunicação ou alteração confirmada de estado).

### 2. Alvo Confirmado

* O código foi gerado, compilado e validado para a arquitetura/placa-alvo de produção (`Target`), e não apenas no ambiente de simulação ou computador do desenvolvedor (`Host`).

### 3. Toolchain/SDK Confirmados

* A compilação utilizou a versão exata do compilador (GCC, Clang, IAR), parâmetros de otimização (`-O2`, `-Os`) e versão de SDK especificados no projeto.

### 4. Reset Considerado

* O sistema inicia em estado seguro após qualquer tipo de reinicialização (*Power-On Reset*, *Software Reset*, *Brown-out Reset*). Variáveis e periféricos são inicializados corretamente sem dependência de estados residuais de memória.

### 5. Watchdog Considerado

* O fluxo de execução alimenta o Watchdog Timer (WDT) nos intervalos corretos. Operações de longa duração não causam resets indesejados e o travamento do código é capturado corretamente pelo WDT.

### 6. Timing Considerado

* Restrições de tempo real foram respeitadas. Não foram introduzidas chamadas bloqueantes inapropriadas (como `delays` longos) que causem violação de *deadlines* ou aumentem a latência de interrupções (ISRs).

### 7. RAM e Flash Considerados

* A alocação de memória foi auditada. A implementação não causa estouro de memória Flash e respeita os limites de pilha (*Stack*) e Heap da RAM, prevenindo colisões de *Stack Overflow*.

### 8. Energia Considerada

* O perfil de consumo elétrico foi mantido dentro do especificado. Periféricos não utilizados são desativados ou colocados em modos de baixa energia (*Sleep/Stop*).

### 9. Testes Executados no Nível Correto

* A alteração foi submetida ao nível de teste adequado à sua complexidade (testes unitários no *Host*, testes de integração no *Hardware-in-the-Loop* ou ensaios de bancada).

### 10. Resultado Registrado

* Provas concretas do funcionamento foram anexadas ao repositório ou rastreador de tarefas (logs do console, capturas de analisador lógico/osciloscópio, relatórios de teste).