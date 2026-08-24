# CONCERNS

## Riscos e mitigação

| Risco | Impacto | Mitigação |
|---|---|---|
| DS18B20 desconectado/falho | Temperatura inválida | Marcar erro no JSON; dashboard degrada, mantém A0 e HTTP ativos |
| RAM insuficiente (strings) | Crash | Dashboard em PROGMEM; evitar alocação dinâmica |
| Wi-Fi aberto (sem encriptação) | Acesso à rede | Aceito por requisito da palestra; sem dados sensíveis |
| ADC fora de faixa | Leitura incorreta | Validar 0..1023; documentar tensão real A CONFIRMAR |
| Watchdog não testado | Reset não detectado | Implementar `ESP.wdtFeed()`; teste em BANCADA |
| Clock 80 vs 160 MHz | Timing | Fixar 80 MHz no `platformio.ini`; documentar |

## Perguntas bloqueadoras

- Tensão/divisor do A0 no NodeMCU V2 (entrada 0–3,3 V): A CONFIRMAR.
- Necessidade de pull-up externo no DS18B20: A CONFIRMAR (padrão NodeMCU tem pull-up interno configurável via OneWire).
- Capacidade de flash utilizada pelo layout: A CONFIRMAR no build.
