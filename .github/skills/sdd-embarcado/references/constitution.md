# Constituição do Projeto Embarcado

> Copie este arquivo para `.specs/project/constitution.md` e adapte os valores entre colchetes. A constituição contém princípios estáveis do projeto; requisitos específicos de uma feature pertencem a `features/<recurso>/spec.md`.

## Identidade do alvo

- Produto/sistema: [nome]
- MCU e variante: [fabricante, família, part number]
- Placa/revisão: [placa e revisão]
- Toolchain/SDK/RTOS: [versões fixadas]
- Clock e alimentação: [valores e tolerâncias]
- Ambientes de validação disponíveis: [host, simulador, bancada, HIL]

## Princípios obrigatórios

### 1. Segurança e estado seguro

- Em falhas de sensor, comunicação, alimentação ou software, o sistema deve assumir [estado seguro definido pelo produto].
- Atuadores perigosos devem ter [intertravamento, timeout, watchdog ou mecanismo equivalente].
- Nenhuma alteração pode contornar proteção elétrica, limite operacional ou requisito de segurança sem decisão registrada.

### 2. Determinismo e concorrência

- ISR deve ser curta, não bloquear e transferir trabalho para [fila/task/loop] quando aplicável.
- Acesso compartilhado entre ISR, loop e tasks deve usar [política de sincronização] e ownership explícito.
- Cada caminho de tempo real deve declarar período/frequência, deadline e jitter máximo: [valores ou `A DEFINIR`].
- Alocação dinâmica é [proibida/restrita/permitida] em [contextos].

### 3. Recursos limitados

- Limite de flash: [valor]; limite de RAM: [valor]; margem mínima esperada: [valor].
- Stack deve ser medida ou estimada para [tasks/interrupts] e não pode exceder [critério].
- O uso de energia deve respeitar [modos, corrente, duty cycle e método de medição].
- Persistência deve considerar integridade após perda de energia e limite de ciclos de escrita.

### 4. Interfaces de hardware e comunicação

- Pinos, polaridade, níveis elétricos, unidades, escalas e faixas devem ser documentados antes da implementação.
- Protocolos devem definir framing, endianess, timeout, CRC, retries, versão e comportamento para mensagens inválidas.
- Alterações de pinagem, clock, protocolo ou formato persistente exigem revisão de compatibilidade.

### 5. Qualidade e rastreabilidade

- Todo requisito funcional recebe um ID `FR-###`; todo requisito não funcional recebe `NFR-###`.
- Cada requisito alterado possui teste ou evidência; pendências devem registrar risco residual e responsável.
- O build deve ser reproduzível com versões registradas e warnings tratados conforme [política].
- Código gerado por ferramenta não substitui revisão de contrato, segurança e comportamento no alvo.

### 6. Diagnóstico e recuperação

- Logs, códigos de erro e telemetria devem respeitar [formato, taxa, privacidade e custo de memória].
- Watchdog, brownout, reset e atualização devem deixar evidência suficiente para diagnóstico sem impedir o estado seguro.
- A política de recuperação após falha é: [reiniciar módulo, degradar, permanecer seguro, exigir intervenção].

### 7. Processo de mudança

- A especificação é atualizada quando o comportamento aprovado muda; não se aceita corrigir apenas o código deixando o contrato obsoleto.
- Decisões que alteram risco, arquitetura, timing, energia, memória ou compatibilidade são registradas em `STATE.md` ou no design da feature.
- Uma tarefa deve ser pequena o bastante para revisão e verificação isoladas.
- Não se adicionam abstrações, dependências ou camadas sem benefício verificável no alvo.

## Gates padrão

- [ ] Requisitos e critérios são observáveis e possuem IDs.
- [ ] Alvo, versão de toolchain e dependências foram confirmados.
- [ ] Caminhos de erro, reset, watchdog e estado seguro foram considerados.
- [ ] Timing, concorrência, RAM, flash e energia foram avaliados quando aplicáveis.
- [ ] Testes foram executados no nível declarado: `HOST`, `SIMULADOR`, `BANCADA` ou `HIL`.
- [ ] O resultado e as limitações estão registrados.
