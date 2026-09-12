// ds18b20_sensor.cpp — implementação do driver do DS18B20.
#include "ds18b20_sensor.h"

#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>

#include <string.h>

#include "config.h"

namespace sensor {
namespace {

OneWire g_wire(cfg::kPinOneWire);
DallasTemperature g_ds(&g_wire);
bool g_present = false;      // presença observada na última varredura
uint8_t g_addr[8] = {0};     // ROM vinculada na varredura (FR-001)
bool g_addr_valid = false;   // vínculo utilizável para conversão/leitura (DEC-07)

// Faixa física do DS18B20 (datasheet). Fora dela (ou -127/NaN) => leitura inválida.
constexpr float kMinC = -55.0f;
constexpr float kMaxC = 125.0f;

void logAddress(const uint8_t* addr) {
  char buf[24];
  snprintf(buf, sizeof(buf), "%02X%02X%02X%02X%02X%02X%02X%02X", addr[0], addr[1], addr[2],
           addr[3], addr[4], addr[5], addr[6], addr[7]);
  Serial.printf("[sensor] DS18B20 detectado (ROM %s) parasita=%d\n", buf,
                g_ds.isParasitePowerMode() ? 1 : 0);
}

// Re-enumera o barramento e (re)vincula a ROM do DS18B20 (FR-001/FR-003).
// O `getDeviceCount()` do DallasTemperature é cacheado e NÃO re-varre: a busca
// real exige `begin()`, que reinicia a enumeração do zero (ADR-013).
// A primeira transação após o reset do MCU pode falhar (barramento ainda
// acomodando o nível alto); tentativas imediatas adicionais recuperam o vínculo
// sem espera ativa — 3 no boot e 2 nas re-varreduras (ADR-014).
bool enumerate(bool boot) {
  uint8_t found[8];
  bool ok = false;
  const uint8_t attempts = boot ? 3 : 2;
  for (uint8_t i = 0; i < attempts && !ok; ++i) {
    g_ds.begin();
    ok = g_ds.getDeviceCount() > 0 && g_ds.getAddress(found, 0);
  }
  if (!ok) {
    if (g_present) {
      Serial.println("[sensor] sensor ausente — carga bloqueada (FR-003)");
    } else if (boot) {
      Serial.println("[sensor] nenhum DS18B20 detectado — carga bloqueada (FR-002)");
    }
    g_present = false;
    g_addr_valid = false;
    return false;
  }

  const bool changed = !g_addr_valid || memcmp(found, g_addr, sizeof(g_addr)) != 0;
  memcpy(g_addr, found, sizeof(g_addr));  // vínculo de ROM usado pelas leituras
  g_addr_valid = true;
  if (changed) {
    g_ds.setResolution(12);  // 12 bits também em sensor substituído (~750 ms)
    logAddress(found);
  }
  if (!g_present && !boot) {
    Serial.println("[sensor] sensor detectado em runtime — retomando leituras (FR-003)");
  }
  g_present = true;
  return true;
}

}  // namespace

void begin() {
  // Mitigação elétrica (ADR-014): habilita o pull-up INTERNO do GPIO4 no barramento.
  // O core ESP8266 só liga o pull-up em `INPUT_PULLUP` (`INPUT` não liga) e a lib
  // OneWire usa `INPUT`; sem pull-up externo o barramento fica flutuante e as
  // leituras falham de forma intermitente. O resistor externo de 4,7 kΩ segue
  // recomendado (P3). As trocas de modo da lib são a nível de registrador e não
  // desligam este pull-up.
  pinMode(cfg::kPinOneWire, INPUT_PULLUP);
  g_ds.setWaitForConversion(false);  // conversão assíncrona (FR-005)
  enumerate(true);                   // varredura inicial + vínculo da ROM (FR-001)
}

bool present() { return g_present; }

bool rescan() { return enumerate(false); }  // re-varredura de 5 s (FR-003)

void startConversion() {
  if (g_addr_valid) {
    g_ds.requestTemperaturesByAddress(g_addr);  // assíncrono: retorna imediatamente
  }
}

Reading read() {
  if (!g_addr_valid) {
    return {false, 0};
  }
  // Uma leitura pode falhar por glitch isolado do barramento; 1 retry imediato
  // absorve o transiente sem espera ativa (ADR-014). Se falhar de novo, a
  // leitura é inválida e a política bloqueia a carga (FR-006/FR-014).
  for (uint8_t i = 0; i < 2; ++i) {
    const float t = g_ds.getTempC(g_addr);  // leitura pela ROM vinculada (DEC-07)
    if (!(isnan(t) || t == DEVICE_DISCONNECTED_C || t < kMinC || t > kMaxC)) {
      return {true, static_cast<int16_t>(lroundf(t * 100.0f))};
    }
  }
  return {false, 0};
}

}  // namespace sensor
