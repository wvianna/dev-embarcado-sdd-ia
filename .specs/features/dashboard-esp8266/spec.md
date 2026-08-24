# Dashboard Web Embarcado — ESP8266 (spec)

## Objetivo

Implementar um firmware para NodeMCU V2 (ESP8266MOD) que adquire temperatura (DS18B20 em D2/GPIO4) e sinal analógico (A0, 0 a 1023) a 1 Hz e os apresenta em um dashboard web servido pelo próprio MCU, operando em modo Access Point com IP fixo `192.168.4.1`.

## Fora de escopo

- MQTT, nuvem ou integração externa.
- Persistência em flash/EEPROM.
- Modo estação (STA).
- Baixo consumo/sleep.
- Atuadores e controle.

## Requisitos funcionais

### FR-001 — Leitura de temperatura

O firmware deve ler a temperatura do DS18B20 (OneWire, GPIO4/D2) em cada ciclo de aquisição e convertê-la para graus Celsius.

### FR-002 — Scan do barramento OneWire

O firmware deve executar, durante a inicialização, um scan do barramento OneWire no GPIO4/D2, enumerar os endereços ROM de 64 bits encontrados e identificar como DS18B20 os dispositivos cuja família seja `0x28`.

### FR-003 — Registro do endereço do sensor

O firmware deve selecionar o primeiro DS18B20 encontrado, armazenar seu endereço ROM para as leituras seguintes e registrar o endereço no log serial em formato hexadecimal.

### FR-004 — Indicação numérica

O dashboard deve exibir numericamente a temperatura em graus Celsius e o valor bruto de A0. O timestamp permanece disponível no JSON, mas não deve ser apresentado em um quadro separado.

### FR-005 — Gauge de temperatura

O dashboard deve exibir a temperatura em um gauge com escala fixa de 20 a 40 °C, unidade em graus Celsius e indicação visual de erro quando `sensor_error` for `true`.

### FR-006 — Gráfico de tendência

O dashboard deve exibir um gráfico de tendência da temperatura com escala fixa de 20 a 40 °C, adicionar um ponto a cada ciclo de aquisição e manter uma janela histórica limitada.

### FR-007 — Gauge do sinal analógico

O dashboard deve exibir o valor de A0 em um gauge com escala fixa de 0 a 1023.

### FR-008 — Gráfico de tendência do sinal analógico

O dashboard deve exibir um gráfico de tendência do A0 com escala fixa de 0 a 1023, adicionar um ponto a cada ciclo de aquisição e usar a mesma janela histórica limitada da temperatura.

### FR-009 — Leitura analógica

O firmware deve ler o pino A0 a cada ciclo de aquisição e manter o valor bruto inteiro na faixa 0 a 1023.

### FR-010 — Periodicidade

O firmware deve executar a aquisição de temperatura e A0 a cada 1 s.

### FR-011 — Estado da amostra

O firmware deve manter a amostra mais recente (temperatura, ADC, timestamp e flag de erro do sensor) disponível para o servidor web.

### FR-012 — Dashboard web

O firmware deve servir uma página HTML com o dashboard em `GET /`, exibindo temperatura, valor de A0 e timestamp da última atualização, com atualização automática sem recarregar a página.

### FR-013 — Endpoint JSON

O firmware deve servir `GET /api/values` retornando um JSON com `temperature`, `adc`, `timestamp` e `sensor_error`.

### FR-014 — Modo AP

O firmware deve iniciar um Access Point com SSID derivado do endereço MAC (ex.: `ESP8266-1A2B3C` usando os 3 últimos bytes), rede aberta (sem encriptação).

### FR-015 — IP fixo e DHCP

O AP deve usar IP fixo `192.168.4.1`, gateway `192.168.4.1`, máscara `255.255.255.0`, sub-rede `192.168.4.0/24`, com DHCP entregando endereços aos clientes.

### FR-016 — Falha do sensor

Em falha de leitura do DS18B20, o firmware deve indicar erro no dashboard/JSON e continuar respondendo às requisições web e à leitura do A0.

## Requisitos não funcionais

### NFR-001 — Timing

A aquisição deve ocorrer a 1 Hz, com deadline máximo de 1,1 s e jitter de ±100 ms.

### NFR-002 — Recursos

O firmware deve operar dentro da RAM disponível do ESP8266 sem alocação dinâmica no caminho periódico; strings do dashboard em `PROGMEM`.

### NFR-003 — Watchdog

O firmware deve alimentar o watchdog (via `ESP.wdtFeed()`) para evitar reset espúrio durante operação normal.

### NFR-004 — Serial

Logs em 115200 baud, prefixados por nível (INFO/WARN/ERROR), curtos e estáveis.

### NFR-005 — Escala do ADC

A leitura de A0 deve permanecer no intervalo 0 a 1023 (inteiro, sem conversão de tensão nesta versão).

## Critérios de aceitação

- **CA-001:** DADO o MCU energizado QUANDO o AP iniciar ENTÃO um SSID derivado do MAC deve ficar visível e aberto.
- **CA-002:** DADO um cliente conectado ao AP QUANDO o DHCP responder ENTÃO o cliente recebe um IP da sub-rede `192.168.4.0/24`.
- **CA-003:** DADO o MCU energizado QUANDO o scan OneWire terminar ENTÃO cada ROM encontrada deve ser registrada no log e uma ROM de família `0x28` deve ser identificada como DS18B20.
- **CA-004:** DADO um DS18B20 encontrado QUANDO iniciar a aquisição ENTÃO o endereço ROM identificado deve ser usado para ler a temperatura.
- **CA-005:** DADO o servidor ativo QUANDO acessar `http://192.168.4.1` ENTÃO o dashboard deve carregar e exibir numericamente temperatura e A0, sem quadro separado de última atualização.
- **CA-006:** DADO uma temperatura válida QUANDO o dashboard for exibido ENTÃO o gauge de temperatura deve usar escala fixa de 20 a 40 °C.
- **CA-007:** DADO uma leitura de A0 QUANDO o dashboard for exibido ENTÃO o gauge analógico deve usar escala fixa de 0 a 1023.
- **CA-008:** DADO o dashboard aberto QUANDO novos dados forem adquiridos ENTÃO a página deve atualizar automaticamente (fetch periódico) sem recarregar.
- **CA-009:** DADO o dashboard aberto QUANDO novos dados forem adquiridos ENTÃO novos pontos devem ser adicionados aos gráficos de temperatura e A0, respeitando a janela histórica limitada.
- **CA-010:** DADO o endpoint `GET /api/values` QUANDO solicitado ENTÃO deve retornar JSON válido com `temperature`, `adc`, `timestamp` e `sensor_error`.
- **CA-011:** DADO o ADC operando QUANDO a leitura for realizada ENTÃO o valor deve permanecer entre 0 e 1023.
- **CA-012:** DADO o DS18B20 com falha ou ausente QUANDO a aquisição ocorrer ENTÃO `sensor_error` deve ser `true`, o gauge de temperatura deve indicar erro e o servidor web deve permanecer responsivo.

## Premissas e riscos

- NodeMCU V2 fornece 3,3 V e possui pull-up interno suficiente para o DS18B20 em OneWire (A CONFIRMAR em bancada).
- Tensão do ADC assume entrada 0–3,3 V com divisor da placa (A CONFIRMAR).
- Validação em BANCADA pendente de hardware físico.
- O scan atual encontra um sensor DS18B20 com ROM `28FFE203B41605C2` na bancada utilizada; outros endereços devem ser descobertos dinamicamente.
