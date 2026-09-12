// buzzer_pattern.cpp — implementação do padrão do buzzer.
#include "buzzer_pattern.h"

namespace thermal {

void BuzzerPattern::begin(uint32_t now_ms) {
  active_ = false;
  start_ms_ = now_ms;
}

bool BuzzerPattern::update(uint32_t now_ms, bool condition) {
  if (!condition) {
    active_ = false;  // alarme cessou: buzzer desligado imediatamente
    return false;
  }
  if (!active_) {
    active_ = true;
    start_ms_ = now_ms;  // começa sempre na fase ON (150 ms)
  }
  const uint32_t elapsed = static_cast<uint32_t>(now_ms - start_ms_);  // wrap-safe
  return (elapsed % kPeriodMs) < kOnMs;
}

}  // namespace thermal
