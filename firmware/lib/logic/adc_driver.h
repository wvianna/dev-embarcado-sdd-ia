#ifndef ADC_DRIVER_H
#define ADC_DRIVER_H

#include <Arduino.h>

// Leitura do pino analógico A0 (0 a 1023) do ESP8266.
class AdcDriver {
public:
    // Pino analógico do ESP8266 (TOUT / A0). Usa analogRead().
    explicit AdcDriver(uint8_t pin = A0);

    // Retorna o valor bruto 0..1023. Fora da faixa indica problema de hardware.
    uint16_t read();

private:
    uint8_t _pin;
};

#endif // ADC_DRIVER_H
