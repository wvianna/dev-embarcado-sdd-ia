#include "json_formatter.h"
#include <ArduinoJson.h>

String formatValuesJson(const Sample& s) {
    JsonDocument doc;
    if (s.sensorError || isnan(s.temperature)) {
        doc["temperature"] = nullptr;
        doc["sensor_error"] = true;
    } else {
        doc["temperature"] = s.temperature;
        doc["sensor_error"] = false;
    }
    doc["adc"] = s.adc;
    doc["timestamp"] = s.timestamp;

    // Buffer fixo: evita alocação dinâmica no caminho HTTP (NFR-002).
    char buf[160];
    serializeJson(doc, buf, sizeof(buf));
    return String(buf);
}
