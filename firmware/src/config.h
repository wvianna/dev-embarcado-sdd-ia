// config.h — constantes únicas do firmware (pinos, tempos, limiares, rede, memória).
//
// Fonte de verdade das constantes de integração; a lógica pura declara as suas
// próprias e o static_assert abaixo falha a compilação se divergirem (design §4.1).
#pragma once

#include <Arduino.h>

#include "buzzer_pattern.h"
#include "control_policy.h"

namespace cfg {

// --- Pinagem fixa (AGENTS.md §5 / constitution §4 — não alterar) ---
constexpr uint8_t kPinOneWire = 4;  // D2/GPIO4 — DS18B20 (OneWire)
constexpr uint8_t kPinBuzzer = 16;  // D0/GPIO16 — buzzer ativo (somente digital)
constexpr uint8_t kPinHeater = 5;   // D1/GPIO5 — resistência (PWM 0–1023)

// --- Tempos e limiares ---
constexpr uint16_t kHeaterPwmMax = 1023;    // analogWriteRange(1023) — 10 bits
constexpr int16_t kSafetyTempCenti = 8000;  // >= 80,00 °C (FR-010)
constexpr uint32_t kSamplePeriodMs = 1200;  // amostragem nominal (FR-004; DEC-07)
constexpr uint32_t kSensorRescanMs = 5000;  // re-varredura OneWire (FR-003)
constexpr uint32_t kBuzzerOnMs = 150;       // FR-013
constexpr uint32_t kBuzzerOffMs = 2000;     // FR-013
constexpr uint32_t kHealthWindowMs = 1000;  // janela da métrica load/idle (design §5.1)
constexpr uint32_t kSerialBaud = 115200;

// --- Rede (FR-018/FR-019) ---
constexpr uint16_t kHttpPort = 80;
constexpr uint8_t kApIp[4] = {192, 168, 4, 1};
constexpr uint8_t kApMask[4] = {255, 255, 255, 0};

// --- Memória estática (NFR-002) ---
constexpr size_t kTrendCapacity = 120;  // 144 s de histórico a 1,2 s (DEC-07)
constexpr size_t kJsonBufSize = 1600;   // payload /json (contrato design §3.5)
constexpr uint32_t kSketchSpaceBytes = 1044464;  // espaço de sketch do env nodemcuv2

// --- Identificação ---
constexpr char kFwVersion[] = "1.0.0";

// Consistência lógica pura x integração (falha em tempo de compilação).
static_assert(thermal::ControlPolicy::kSafetyTempCenti == kSafetyTempCenti,
              "limiar de segurança divergente entre config.h e ControlPolicy");
static_assert(thermal::ControlPolicy::kPwmMax == kHeaterPwmMax,
              "PWM divergente entre config.h e ControlPolicy");
static_assert(thermal::BuzzerPattern::kOnMs == kBuzzerOnMs,
              "fase ON do buzzer divergente entre config.h e BuzzerPattern");
static_assert(thermal::BuzzerPattern::kOffMs == kBuzzerOffMs,
              "fase OFF do buzzer divergente entre config.h e BuzzerPattern");

}  // namespace cfg
