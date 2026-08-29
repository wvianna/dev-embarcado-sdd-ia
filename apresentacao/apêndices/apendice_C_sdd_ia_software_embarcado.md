Aqui está um guia em Markdown explicando os 9 conceitos fundamentais de um **RTOS (Real-Time Operating System)** para desenvolvimento de sistemas embarcados.

---

# Conceitos Fundamentais de RTOS (Sistemas Operacionais em Tempo Real)

## 1. Task / Thread

Uma **Task** (ou Thread) é uma unidade autônoma de execução de código que roda dentro do sistema. Cada task possui seu próprio loop de execução, seu próprio estado (Pronta, Rodando, Bloqueada) e sua própria pilha de memória (**Stack**).

* **Para que serve:** Quebrar um sistema complexo em múltiplos módulos independentes (ex: leitura de sensores, display, conectividade).

## 2. Prioridade

É um valor numérico atribuído a cada Task para indicar sua importância no sistema.

* **Para que serve:** Determinar quem tem a preferência da CPU. Se uma Task de alta prioridade precisa rodar, o sistema interrompe imediatamente qualquer Task de prioridade menor (preempção).

## 3. Scheduler (Escalonador)

O **Scheduler** é o "gerente" do RTOS. É o algoritmo responsável por gerenciar a CPU e decidir qual Task da lista de execução deve rodar em determinado momento.

* **Para que serve:** Garantir determinismo e garantir que a Task pronta de maior prioridade sempre receba o tempo de processamento da CPU.

## 4. Fila (Queue)

Mecanismo de comunicação inter-processos (IPC) baseado no modelo FIFO (*First In, First Out*).

* **Para que serve:** Enviar dados com segurança entre duas Tasks ou entre uma ISR (interrupção) e uma Task, copiando o valor de uma origem para um destino sem corrupção de memória.

## 5. Semáforo

Um sinalizador baseado em contagem ou estado binário (0 ou 1) usado para sincronização de eventos de hardware ou software.

* **Tipos principais:**
* **Binário:** Sinaliza se um evento aconteceu (ex: interrupção do botão avisa a task de tratamento).
* **Contador:** Gerencia o acesso a um pool de recursos limitados (ex: até 3 conexões simultâneas).



## 6. Mutex (Mutual Exclusion)

Um tipo especial de semáforo projetado exclusivamente para proteção de recursos compartilhados (como barramentos I2C/SPI, variáveis globais ou displays).

* **Diferencial:** Possui o conceito de *ownership* (apenas quem pegou o mutex pode soltá-lo) e mecanismo de **Herança de Prioridade** para evitar a *Inversão de Prioridade*.

## 7. Evento (Event Groups / Event Flags)

Um grupo de bits de sinalização (flags) onde cada bit representa que uma determinada condição ocorreu no sistema.

* **Para que serve:** Permite que uma Task aguarde por múltiplas condições ao mesmo tempo (ex: "Acorde apenas se `[Botão Pressionado]` AND `[Dado Recebido na UART]`").

## 8. Timeout

O tempo máximo que uma Task aceita ficar no estado **Bloqueado** aguardando por um recurso (uma Fila, Mutex, Semáforo ou Evento).

* **Para que serve:** Evitar que o sistema trave indefinidamente se um hardware falhar ou uma mensagem nunca chegar. Se o tempo limite expirar, a função retorna um código de erro que o código trata.

## 9. Stack (Pilha da Task)

Uma área contígua de memória RAM reservada individualmente para **cada Task**.

* **Para que serve:** Armazenar variáveis locais, endereços de retorno de chamadas de funções e o contexto dos registradores da CPU durante as trocas de contexto.
* **Atenção:** Se a Stack for dimensionada com tamanho menor do que o uso real da Task, ocorre o fatal **Stack Overflow**.