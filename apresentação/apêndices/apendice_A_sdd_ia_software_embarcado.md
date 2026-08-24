# Apêndice — Se a IA escreve o código, o programador acabou?

## 1. Uma provocação

### “Se a IA escreve o código, o programador acabou?”

**Não. O papel muda.**

Em software embarcado, escrever código é apenas uma parte do problema de engenharia.

Imagine que alguém peça a uma IA:

> “Faça um sistema que leia um sensor de temperatura e acione um ventilador.”

A IA consegue gerar uma implementação bastante convincente. Porém, a partir dessa frase, ela não sabe necessariamente:

- qual sensor está sendo utilizado;
- qual é sua faixa de operação;
- qual a precisão necessária;
- qual ADC está conectado;
- qual tensão de referência existe;
- qual é o tempo máximo aceitável para uma leitura;
- o que fazer se o sensor for desconectado;
- qual temperatura representa uma condição perigosa;
- se o ventilador deve desligar em caso de falha;
- quanto de RAM pode ser utilizado;
- qual prioridade uma tarefa deve possuir no RTOS.

Essas são **decisões de engenharia**.

A IA pode ajudar a implementá-las, mas alguém precisa **defini-las, justificá-las e validá-las**.

---

## 2. Quem definiu o comportamento que o código deve implementar?

Essa é uma das perguntas centrais do desenvolvimento assistido por IA.

Considere o requisito:

> “Acione o ventilador quando a temperatura ultrapassar 70 °C.”

Parece simples, mas imediatamente surgem outras perguntas.

E se a temperatura ficar oscilando entre 69,9 °C e 70,1 °C?

O ventilador pode ficar:

```text
liga → desliga → liga → desliga → ...
```

Uma solução poderia ser utilizar **histerese**:

```text
70 °C ───── liga
65 °C ───── desliga
```

Agora existe um comportamento muito mais bem definido.

A IA pode implementar a lógica de histerese muito bem. Porém, a decisão de utilizar 70 °C para ligar e 65 °C para desligar é uma **decisão de engenharia**.

A pergunta fundamental passa a ser:

> **O que exatamente o sistema deve fazer?**

Essa resposta deve estar registrada em requisitos, especificações e critérios de aceitação.

---

## 3. Quais são os limites?

Software embarcado vive cercado de limites físicos e computacionais.

Por exemplo:

```text
Sensor:
- alimentação: 3,3 V
- faixa: -40 a 125 °C
- precisão: ±0,5 °C

MCU:
- RAM: 128 KB
- Flash: 512 KB
- CPU: 120 MHz

Comunicação:
- CAN: 500 kbit/s
- UART: 115200 baud
```

Considere ainda o requisito:

> “O sistema deve responder a um evento em menos de 100 µs.”

A IA pode gerar um código funcional que responde em 500 µs.

O código pode:

- compilar;
- passar nos testes funcionais;
- parecer correto.

Mas **não atende ao requisito temporal**.

Por isso, em sistemas embarcados:

> **“Funciona” não significa necessariamente “está correto”.**

É preciso conhecer e verificar os limites do sistema.

---

## 4. O que acontece quando o sensor falha?

Imagine:

```text
Sensor → MCU → Controle → Atuador
```

O sensor é desconectado.

O que o software deve fazer?

Possibilidades:

1. manter o último valor;
2. assumir temperatura zero;
3. assumir temperatura máxima;
4. desligar o equipamento;
5. entrar em estado seguro;
6. gerar um alarme;
7. tentar recuperar o sensor;
8. reinicializar o periférico.

Não existe uma resposta universal.

**É uma decisão de projeto.**

Uma especificação poderia dizer:

> “Se a comunicação com o sensor permanecer indisponível por mais de 100 ms, o controlador deverá entrar em estado seguro e desligar o atuador.”

Agora a IA possui algo concreto para implementar e testar.

---

## 5. O que acontece no reset?

Em um microcontrolador, o reset é parte fundamental do comportamento do sistema.

Um fluxo típico pode ser:

```text
POWER ON
   ↓
RESET
   ↓
BOOT
   ↓
Inicialização
   ↓
Periféricos
   ↓
RTOS
   ↓
Aplicação
```

O engenheiro precisa decidir, entre outras coisas:

- GPIO começa como entrada ou saída?
- O atuador começa desligado?
- O watchdog é habilitado imediatamente?
- A RAM precisa ser inicializada?
- O estado anterior precisa ser recuperado da Flash?
- O sistema pode liberar um motor antes de inicializar os sensores?
- Quais periféricos precisam estar disponíveis antes da aplicação iniciar?

Imagine uma GPIO conectada a uma válvula.

Se a configuração inicial estiver inadequada:

```text
RESET
  ↓
GPIO em estado inadequado
  ↓
VÁLVULA ABRE
```

O problema pode ocorrer antes mesmo de a aplicação estar totalmente inicializada.

Portanto:

> **O comportamento durante reset e inicialização também deve fazer parte da especificação.**

---

## 6. Quanto de RAM pode ser usado?

Uma IA pode produzir código perfeitamente válido do ponto de vista da linguagem, mas que excede os recursos do MCU.

Exemplo:

```text
RAM disponível: 128 KB

Stack:        32 KB
RTOS:         20 KB
Buffers:      40 KB
Drivers:      10 KB
Aplicação:    20 KB
--------------------
Total:       122 KB
```

A aplicação pode compilar.

Mas uma nova funcionalidade pode adicionar:

```c
uint8_t buffer[16 * 1024];
```

e provocar estouro ou comportamento imprevisível.

Por isso, um requisito poderia estabelecer:

> “O consumo máximo de RAM da aplicação não deve exceder 80 KB.”

A IA pode ajudar a medir, analisar e otimizar o consumo, mas **o limite é uma decisão de engenharia**.

---

## 7. Quanto de Flash pode ser consumido?

O mesmo raciocínio vale para a memória Flash.

Uma IA pode escolher uma biblioteca extremamente conveniente:

```text
Biblioteca X
↓
+180 KB
```

Ela funciona.

Mas:

```text
Flash disponível: 256 KB
Firmware atual:   240 KB
```

A próxima funcionalidade pode tornar o firmware inviável.

Um requisito poderia estabelecer:

> “O firmware deverá utilizar no máximo 80% da memória Flash disponível.”

Esse requisito pode inclusive ser transformado em um **quality gate automatizado** do processo de desenvolvimento.

---

## 8. Qual é o estado seguro?

Essa é uma das perguntas mais importantes em sistemas embarcados.

Imagine um sistema controlando:

- motor;
- bomba;
- válvula;
- aquecedor;
- relé;
- freio;
- fonte de potência.

Se alguma coisa falhar:

```text
Sensor
   ↓
Comunicação
   ↓
RTOS
   ↓
Software
   ↓
MCU
```

**O que o sistema deve fazer?**

Em determinado sistema, o estado seguro pode ser:

```text
Motor    → OFF
Válvula  → FECHADA
Bomba    → OFF
Aquecedor → OFF
```

Mas em outro sistema, o estado seguro pode ser justamente o contrário.

Uma bomba de refrigeração, por exemplo, pode precisar permanecer ligada em determinadas condições.

Portanto:

> **“Estado seguro” não é uma decisão da IA. É uma decisão de engenharia.**

A IA precisa receber essa decisão como requisito ou restrição.

---

## 9. Como provar que o código funciona no alvo?

Essa é uma das maiores diferenças entre software convencional e software embarcado.

A IA pode gerar:

```text
driver.c
driver.h
test_driver.c
```

E os testes unitários podem apresentar:

```text
PASS
PASS
PASS
```

Mas isso não prova necessariamente que o sistema funciona no hardware real.

Pode ocorrer:

```text
                 IA
                  │
                  ▼
               Código
                  │
         ┌────────┴────────┐
         ▼                 ▼
     Unit Test          Hardware
         │                 │
       PASS               FAIL
```

Possíveis causas:

- timing diferente;
- interrupção configurada incorretamente;
- registrador incorreto;
- clock configurado incorretamente;
- problema de DMA;
- race condition;
- ruído elétrico;
- problema de alimentação;
- diferença entre simulador e hardware;
- interação inesperada entre periféricos.

Por isso, dependendo do projeto, podem ser necessários:

- testes unitários;
- testes de integração;
- análise estática;
- testes de timing;
- testes HIL (*Hardware-in-the-Loop*);
- testes no hardware real;
- medições com osciloscópio;
- análise com analisador lógico;
- análise de consumo;
- testes de watchdog;
- testes de reset;
- testes de falha.

A IA pode ajudar a criar e executar muitos desses testes, mas os critérios de aceitação e as evidências necessárias precisam ser definidos pelo projeto.

---

# 10. Onde entra o SDD?

É justamente nesse ponto que podemos fazer a transição para **Spec-Driven Development**.

Um fluxo tradicional simplificado pode ser representado por:

```text
Programador
     │
     ▼
   Código
     │
     ▼
   Teste
```

Com desenvolvimento assistido por IA, o processo pode evoluir para:

```text
              SPEC
                │
                ▼
          ARCHITECTURE
                │
                ▼
               PLAN
                │
                ▼
              TASK
                │
                ▼
               IA
                │
                ▼
              CODE
                │
        ┌───────┴────────┐
        ▼                ▼
      TESTE          ANÁLISE
        │                │
        └───────┬────────┘
                ▼
             EVIDÊNCIA
```

A mudança de paradigma é:

> **Não estamos mais principalmente dizendo à IA como escrever cada linha de código. Estamos especificando o comportamento, as restrições e os critérios para considerar a implementação correta.**

---

# 11. O papel dos artefatos no desenvolvimento orientado por IA

Nesse contexto, diferentes artefatos possuem funções diferentes:

| Artefato | Pergunta respondida |
|---|---|
| **Spec** | O que deve ser construído? |
| **Architecture** | Como o sistema será organizado? |
| **Plan** | Como o trabalho será executado? |
| **Task** | Qual trabalho deve ser realizado agora? |
| **Rules** | Quais restrições devem sempre ser obedecidas? |
| **Skills** | Como realizar uma determinada atividade especializada? |
| **Context** | Quais informações o agente precisa conhecer? |
| **Examples** | Como este projeto normalmente implementa algo? |
| **Tests** | Como verificar o comportamento? |
| **Acceptance Criteria** | Quando podemos considerar a tarefa concluída? |
| **ADR** | Por que uma decisão importante foi tomada? |
| **Harness** | Quais ferramentas, recursos e mecanismos de feedback o agente possui? |

Esses artefatos permitem que o agente trabalhe dentro de um **processo de engenharia**, em vez de simplesmente responder a prompts isolados.

---

# 12. E por que “ler datasheet” continua sendo essencial?

Uma IA pode ser excelente para auxiliar na interpretação de documentação técnica.

Por exemplo, podemos pedir:

> “Explique como configurar este registrador para habilitar a interrupção.”

> “Quais são as condições necessárias para iniciar uma conversão ADC?”

> “Qual é o tempo mínimo entre essas duas operações?”

> “Explique este diagrama de timing.”

Isso pode acelerar muito o trabalho do engenheiro.

Porém, o engenheiro precisa saber:

1. **qual informação procurar;**
2. **qual documentação é a fonte correta;**
3. **como verificar a interpretação da IA;**
4. **como transformar a informação em requisito ou decisão de projeto;**
5. **como validar o comportamento no hardware.**

Um pequeno erro de interpretação pode resultar em:

```text
bit errado
   ↓
registrador errado
   ↓
periférico não funciona
   ↓
horas de debugging
```

Em sistemas críticos, as consequências podem ser ainda mais graves.

Portanto:

> **A IA pode ajudar a ler e interpretar o datasheet, mas não elimina a necessidade de conhecimento técnico e validação.**

---

# 13. A mensagem principal

Uma forma de resumir toda essa discussão é:

> ### **A IA pode escrever o código. O engenheiro continua responsável por definir o comportamento, as restrições e as evidências necessárias para afirmar que o código está correto.**

E, conectando diretamente com SDD:

> ### **SDD desloca o trabalho do engenheiro de “escrever cada linha de código” para “especificar, orientar, verificar e validar o comportamento do sistema”.**

Essa mudança é particularmente importante em **software embarcado**, porque o código não existe isoladamente.

Ele está ligado a:

```text
          SOFTWARE
              │
    ┌─────────┼─────────┐
    ▼         ▼         ▼
 HARDWARE   TEMPO     MEMÓRIA
    │         │         │
    ▼         ▼         ▼
PERIFÉRICOS  RTOS    RECURSOS
    │         │         │
    └─────────┼─────────┘
              ▼
        SISTEMA FÍSICO
```

Por isso, quanto maior a capacidade da IA de gerar código, **mais importante se torna a capacidade do engenheiro de especificar e verificar o sistema**.

---

## Frase para encerrar o apêndice

> **O futuro do desenvolvimento de software não é “IA versus programador”. É o engenheiro utilizando IA para transformar especificações bem definidas em implementações verificáveis.**

