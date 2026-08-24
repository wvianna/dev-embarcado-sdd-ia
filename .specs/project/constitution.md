# Constituição do Projeto Embarcado

> Constituição preenchida para a demonstração prática do dashboard web embarcado. Requisitos específicos de uma feature pertencem a `features/<recurso>/spec.md`.

## Identidade do alvo

- Produto/sistema: Dashboard web embarcado de temperatura e sinal analógico
- MCU e variante: módulo Espressif ESP8266MOD
- Placa/revisão: NodeMCU V2 (Amica / ESP-12E / ESP8266MOD)
- Toolchain/SDK/RTOS: Arduino Core para ESP8266 (versão >= 3.0.0$, baseado no Non-OS SDK OU Non-Preemptive RTOS interno de 2 tarefas); compilação via PlatformIO
- Clock e alimentação: Clock padrão de 80 MHz (configurável para 160 MHz$ se necessário); Alimentação 5V via Micro-USB ou gpio(regulador interno AMS1117 para 3.3 V na placa).
- Ambientes de validação disponíveis: BANCADA para ESP8266, Wi-Fi, DS18B20 e A0.

## Princípios obrigatórios

### 1. Segurança e estado seguro

- Em falha do DS18B20, o dashboard deve indicar erro de sensor e continuar respondendo às requisições web.
- Em falha de comunicação Wi-Fi ou cliente desconectado, o MCU deve continuar executando a aquisição local sem bloquear o loop principal.
- Não há atuadores perigosos nesta demonstração; essa ausência deve permanecer explícita no design da feature.
- Nenhuma alteração pode contornar proteção elétrica, limite operacional ou requisito de segurança sem decisão registrada.

### 2. Determinismo e concorrência

- ISR deve ser curta, não bloquear e transferir trabalho para o loop ou task quando aplicável.
- Acesso compartilhado entre aquisição, servidor HTTP e loop deve usar ownership explícito e evitar bloqueios prolongados.
- A aquisição de temperatura e A0 deve ocorrer a 1 Hz; deadline máximo de 1,1 segundos e jitter de +- 100 mS.
- Alocação dinâmica é `A DEFINIR`; deve ser evitada no caminho periódico e no atendimento de requisições até decisão registrada.

### 3. Recursos limitados

- Limite de flash: `A CONFIRMAR`; limite de RAM: `A CONFIRMAR`; margem mínima esperada: `A CONFIRMAR`.
- Stack deve ser medida ou estimada para o loop, servidor web e tarefas utilizadas; critério: `A CONFIRMAR`.
- O uso de energia e o método de medição permanecem `A CONFIRMAR`; não há requisito de baixo consumo definido para a demonstração.
- Não há persistência de dados prevista; qualquer inclusão de parâmetros persistentes exige decisão registrada.

### 4. Interfaces de hardware e comunicação

- DS18B20: D2/GPIO4, barramento OneWire, temperatura em graus Celsius; pull-up e nível elétrico `A CONFIRMAR`.
- ADC: A0, leitura bruta de 0 a 1023; tensão correspondente, atenuação e nível elétrico `A CONFIRMAR`.
- Wi-Fi: modo Access Point, sem encriptação, SSID derivado do MAC (exemplo: `ESP8266-1A2B3C`), sub-rede `192.168.4.0/24`, IP fixo do MCU `192.168.4.1`, gateway `192.168.4.1` e máscara `255.255.255.0`.
- DHCP do AP deve fornecer endereços aos clientes na sub-rede `192.168.4.0/24`.
- HTTP deve servir o dashboard em `http://192.168.4.1` e fornecer endpoint JSON para os valores atuais; framing, timeout e comportamento para requisições inválidas permanecem `A DEFINIR`.
- Alterações de pinagem, rede, clock ou formato do endpoint exigem revisão de compatibilidade.

### 5. Qualidade e rastreabilidade

- Todo requisito funcional recebe um ID `FR-###`; todo requisito não funcional recebe `NFR-###`.
- Cada requisito alterado possui teste ou evidência; pendências devem registrar risco residual e responsável.
- O build deve ser reproduzível com versões registradas e warnings tratados conforme política definida após confirmação da toolchain.
- Código gerado por ferramenta não substitui revisão de contrato, segurança e comportamento no alvo.

### 6. Diagnóstico e recuperação

- O dashboard deve exibir temperatura, valor de A0 e timestamp da última atualização; falha do DS18B20 deve ser representada explicitamente.
- Logs, códigos de erro e telemetria devem respeitar formato, taxa, privacidade e custo de memória `A DEFINIR`.
- Watchdog, brownout, reset e atualização devem deixar evidência suficiente para diagnóstico sem impedir o estado seguro.
- A política de recuperação após falha do sensor é degradar apenas a indicação de temperatura, mantendo a leitura de A0 e o servidor web ativos.
- Após reset ou brownout, o MCU deve reinicializar o AP, o servidor HTTP e as aquisições; detalhes de watchdog e brownout permanecem `A CONFIRMAR`.

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
