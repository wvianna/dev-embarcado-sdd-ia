// ds18b20_sensor.h — driver do DS18B20 (OneWire/DallasTemperature, GPIO4).
//
// Varredura no boot vincula a ROM do sensor (FR-001); conversões e leituras
// usam esse endereço (FR-005/DEC-07). Conversão assíncrona (ADR-002): o
// chamador dispara a conversão e lê o resultado só no ciclo seguinte;
// nenhuma chamada aqui aguarda a conversão.
#pragma once

#include <stdint.h>

namespace sensor {

struct Reading {
  bool valid;     // utilizável? (sem erro de comunicação e dentro da faixa física)
  int16_t centi;  // centésimos de °C (definido apenas quando valid)
};

void begin();            // setup do barramento + varredura inicial + vínculo da ROM (FR-001)
bool present();          // última presença observada no barramento
bool rescan();           // nova varredura; atualiza presença e vínculo (FR-003)
void startConversion();  // dispara conversão assíncrona da ROM vinculada (FR-005)
Reading read();          // leitura da ROM vinculada, sem espera (FR-005/FR-006)

}  // namespace sensor
