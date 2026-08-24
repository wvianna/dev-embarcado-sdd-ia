#ifndef SAMPLER_H
#define SAMPLER_H

#include <Arduino.h>
#include <stdint.h>

// Estrutura da amostra mais recente (FR-004).
struct Sample {
    float temperature;    // °C; NAN quando em falha
    uint16_t adc;         // 0..1023
    uint32_t timestamp;   // millis() da aquisição
    bool sensorError;     // true quando DS18B20 falhou
};

// Função de aquisição chamada pelo loop a cada 1 s (FR-003).
// readTemp: callback que retorna temperatura em °C ou NAN em falha.
// readAdc: callback que retorna o valor 0..1023.
inline void acquireSample(Sample& out,
                          float (*readTemp)(bool*),
                          uint16_t (*readAdc)()) {
    bool ok = false;
    const float t = readTemp(&ok);
    out.temperature = t;
    out.adc = readAdc();
    out.timestamp = millis();
    out.sensorError = !ok;
}

#endif // SAMPLER_H
