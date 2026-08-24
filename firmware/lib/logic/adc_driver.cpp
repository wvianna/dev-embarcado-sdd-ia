#include "adc_driver.h"

AdcDriver::AdcDriver(uint8_t pin) : _pin(pin) {}

uint16_t AdcDriver::read() {
    const int raw = analogRead(_pin);
    // Clamp para a faixa especificada 0..1023 (NFR-005).
    if (raw < 0) return 0;
    if (raw > 1023) return 1023;
    return static_cast<uint16_t>(raw);
}
