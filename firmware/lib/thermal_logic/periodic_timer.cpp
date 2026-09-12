// periodic_timer.cpp — implementação do agendador não bloqueante por deadline.
#include "periodic_timer.h"

namespace thermal {

void PeriodicTimer::begin(uint32_t now_ms, uint32_t period_ms) {
  period_ms_ = period_ms;
  deadline_ms_ = now_ms + period_ms;  // overflow de uint32 é intencional (wrap de millis)
}

bool PeriodicTimer::due(uint32_t now_ms) const {
  // Diferença com sinal: (now - deadline) >= 0 => venceu. Correta quando millis()
  // faz wrap (horizonte de ~24 dias) e quando o deadline acabou de passar
  // (now "atrás" do deadline resulta em valor negativo, não em número gigante).
  return static_cast<int32_t>(now_ms - deadline_ms_) >= 0;
}

bool PeriodicTimer::consume(uint32_t now_ms) {
  if (!due(now_ms)) {
    return false;
  }
  deadline_ms_ += period_ms_;
  // Se, mesmo após avançar, o deadline continua vencido, o loop ficou preso por
  // mais de um período: reancora em now + período (sem rajada de catch-up e sem
  // um segundo disparo no mesmo ms).
  if (due(now_ms)) {
    deadline_ms_ = now_ms + period_ms_;
  }
  return true;
}

}  // namespace thermal
