---
description: Regras de engenharia para firmware e sistemas embarcados.
applyTo: "**/*.c,**/*.h,**/*.cpp,**/*.hpp,**/*.rs,**/*.py,**/*.ino,**/*.S"
---

# Engenharia embarcada

- Não invente pinagem, clock, periféricos, níveis elétricos, temporização ou características do MCU.
- Quando uma informação de hardware estiver ausente, marque `A CONFIRMAR`.
- Separe HAL/BSP de lógica de aplicação sempre que isso melhorar testabilidade.
- Declare contexto de execução: main/loop, ISR, DMA, task/thread.
- Para dados compartilhados entre ISR/DMA/tasks, documente ownership, atomicidade, `volatile`, seção crítica e barreiras quando aplicáveis.
- Nunca faça read-modify-write de registradores sem verificar concorrência e semântica dos bits.
- Trate timeout, reset, watchdog, brownout, comunicação perdida, valores inválidos e recuperação como comportamento especificado.
- Não introduza alocação dinâmica em caminhos críticos sem justificativa.
- Verifique RAM, flash, stack, CPU, timing, jitter, energia e largura de banda quando relevantes.
- Teste lógica pura em HOST quando possível, mas não use teste HOST como substituto de validação no alvo.
