#ifndef JSON_FORMATTER_H
#define JSON_FORMATTER_H

#include <Arduino.h>
#include <sampler.h>

// Gera o payload JSON de GET /api/values (FR-006).
// Retorna uma String pronta para envio HTTP.
String formatValuesJson(const Sample& s);

#endif // JSON_FORMATTER_H
