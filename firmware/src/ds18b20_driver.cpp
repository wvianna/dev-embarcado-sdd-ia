#include "ds18b20_driver.h"
#include <DallasTemperature.h>

static OneWire s_oneWire(4);   // GPIO4 (D2) — barramento compartilhado

Ds18b20Driver::Ds18b20Driver(uint8_t gpio)
    : _gpio(gpio), _error(false), _ready(false), _found(false), _deviceCount(0) {
    memset(_address, 0, sizeof(_address));
}

bool Ds18b20Driver::begin() {
    // OneWire com pinagem dinâmica: reconstrói o barramento se o pino diferir.
    // Para simplificar e manter determinismo, exigimos GPIO4 conforme TARGET.
    if (_gpio != 4) {
        _error = true;
        return false;
    }

    DallasTemperature sensors(&s_oneWire);
    sensors.begin();
    _deviceCount = sensors.getDeviceCount();

    scanAndLog();

    _ready = (_found);
    _error = !_ready;
    return _ready;
}

void Ds18b20Driver::scanAndLog() {
    uint8_t addr[8];
    uint8_t count = 0;
    bool foundTarget = false;

    Serial.println(F("[INFO] Scan OneWire no GPIO4:"));

    // Varredura por ROM (reset + busca) para listar todos os dispositivos.
    while (s_oneWire.search(addr)) {
        Serial.print(F("  - ROM "));
        for (uint8_t i = 0; i < 8; i++) {
            Serial.print(addr[i] < 16 ? F("0") : F(""));
            Serial.print(addr[i], HEX);
        }
        // Byte 0 = família: 0x28 é DS18B20.
        Serial.print(addr[0] == 0x28 ? F(" (DS18B20)") : F(" (outra família)"));
        Serial.println();

        if (!foundTarget && addr[0] == 0x28) {
            memcpy(_address, addr, sizeof(_address));
            foundTarget = true;
        }
        count++;
    }
    s_oneWire.reset_search();

    Serial.print(F("[INFO] Dispositivos OneWire: "));
    Serial.println(count);

    if (foundTarget) {
        _found = true;
        Serial.print(F("[INFO] Sensor alvo (DS18B20) ROM: "));
        for (uint8_t i = 0; i < 8; i++) {
            Serial.print(_address[i] < 16 ? F("0") : F(""));
            Serial.print(_address[i], HEX);
        }
        Serial.println();
    } else {
        _found = false;
        Serial.println(F("[WARN] Nenhum DS18B20 (0x28) encontrado no barramento."));
    }
}

float Ds18b20Driver::readCelsius(bool* ok) {
    if (!_ready || !_found) {
        _error = true;
        if (ok) *ok = false;
        return NAN;
    }

    DallasTemperature sensors(&s_oneWire);
    sensors.requestTemperatures();      // conversão síncrona (~750 ms a 12 bits)
    const float t = sensors.getTempC(_address);

    if (t == DEVICE_DISCONNECTED_C || isnan(t)) {
        _error = true;
        if (ok) *ok = false;
        return NAN;
    }

    _error = false;
    if (ok) *ok = true;
    return t;
}
