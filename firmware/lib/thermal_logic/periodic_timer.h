// periodic_timer.h — agendamento não bloqueante por millis() (lógica pura).
//
// Contrato (design §3.2): baseado em DEADLINE (instante do próximo vencimento),
// com diferença sinalizada — correta perto do overflow de millis() e imune a
// consumos repetidos no mesmo milissegundo. A cadência nominal é mantida sem
// drift; se o vencimento ficar para trás (loop ocupado), o timer reancora em
// `now + período` para não disparar uma rajada de vencimentos em atraso.
#pragma once

#include <stdint.h>

namespace thermal {

class PeriodicTimer {
 public:
  // Ancora o timer: primeiro vencimento em `now_ms + period_ms`.
  void begin(uint32_t now_ms, uint32_t period_ms);

  // O deadline já chegou? (não altera o estado; seguro para consulta)
  bool due(uint32_t now_ms) const;

  // `due` + agendamento do próximo vencimento. Retorna true no máximo uma vez
  // por período (mesmo se o loop consumir várias vezes no mesmo ms).
  bool consume(uint32_t now_ms);

  uint32_t period() const { return period_ms_; }

 private:
  uint32_t deadline_ms_ = 0;  // instante do próximo vencimento
  uint32_t period_ms_ = 0;
};

}  // namespace thermal
