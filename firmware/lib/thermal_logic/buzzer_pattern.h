// buzzer_pattern.h — padrão de sinalização sonora do alarme (lógica pura).
//
// FR-013: 150 ms ligado / 2000 ms desligado apenas enquanto a condição de alarme
// persistir; a saída é o estado FÍSICO do pino (GPIO16, somente digital).
// Reinício de fase ao reativar garante que o ciclo comece em ON (audível).
#pragma once

#include <stdint.h>

namespace thermal {

class BuzzerPattern {
 public:
  static constexpr uint32_t kOnMs = 150;                 // fase ligada
  static constexpr uint32_t kOffMs = 2000;               // fase desligada
  static constexpr uint32_t kPeriodMs = kOnMs + kOffMs;  // ciclo completo (2150 ms)

  void begin(uint32_t now_ms);  // inicia inativo (estado seguro de boot)

  // Atualiza com o tempo atual e a condição de alarme; retorna true se o buzzer
  // deve estar LIGADO neste instante. Sem condição => desligado imediatamente.
  bool update(uint32_t now_ms, bool condition);

 private:
  bool active_ = false;       // condição presente e ciclo em andamento
  uint32_t start_ms_ = 0;     // instante de ativação (fase 0 = ON)
};

}  // namespace thermal
