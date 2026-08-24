#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "ds18b20_driver.h"
#include <adc_driver.h>
#include <sampler.h>
#include "web_server.h"

// ---------------------------------------------------------------------------
// Configuração de rede (constitution.md — seção Interfaces)
// ---------------------------------------------------------------------------
static const IPAddress kApIp(192, 168, 4, 1);
static const IPAddress kApGateway(192, 168, 4, 1);
static const IPAddress kApSubnet(255, 255, 255, 0);

// ---------------------------------------------------------------------------
// Drivers e estado
// ---------------------------------------------------------------------------
static Ds18b20Driver s_temp(4);   // DS18B20 em D2 / GPIO4
static AdcDriver s_adc(A0);
static Sample s_sample{};
static WebServerService s_web(&s_sample);

// Período de aquisição: 1 s (FR-003 / NFR-001)
static const uint32_t kAcquirePeriodMs = 1000;

// ---------------------------------------------------------------------------
// Callbacks para o sampler
// ---------------------------------------------------------------------------
static float readTempCb(bool* ok) {
    return s_temp.readCelsius(ok);
}

static uint16_t readAdcCb() {
    return s_adc.read();
}

// ---------------------------------------------------------------------------
// SSID derivado do MAC (FR-007): ESP8266-<3 últimos bytes>
// ---------------------------------------------------------------------------
static String buildSsid() {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    char buf[32];
    snprintf(buf, sizeof(buf), "ESP8266-%02X%02X%02X", mac[3], mac[4], mac[5]);
    return String(buf);
}

// ---------------------------------------------------------------------------
void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println(F("[INFO] Boot ESP8266 dashboard"));

    // Sensor de temperatura
    s_temp.begin();
    if (s_temp.lastError()) {
        Serial.println(F("[WARN] DS18B20 não encontrado; modo degradado (A0/HTTP ativos)"));
    }

    // Modo AP (FR-007/FR-008)
    const String ssid = buildSsid();
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(kApIp, kApGateway, kApSubnet);
    const bool apOk = WiFi.softAP(ssid.c_str(), /*passphrase=*/nullptr, /*channel=*/1,
                                  /*ssid_hidden=*/0, /*max_connection=*/4);
    Serial.print(F("[INFO] AP: "));
    Serial.println(ssid);
    Serial.print(F("[INFO] AP iniciado: "));
    Serial.println(apOk ? F("sim") : F("não"));
    Serial.print(F("[INFO] IP do AP: "));
    Serial.println(WiFi.softAPIP());

    // Servidor HTTP
    s_web.begin();
    Serial.println(F("[INFO] Servidor HTTP em http://192.168.4.1"));

    // Watchdog alimentado no loop (NFR-003). O framework do Arduino já
    // habilita o watchdog do SDK; mantemos ESP.wdtFeed() no loop.
    ESP.wdtEnable(0);
}

static uint32_t s_lastAcquire = 0;

void loop() {
    const uint32_t now = millis();

    // Aquisição a 1 Hz (FR-003)
    if (now - s_lastAcquire >= kAcquirePeriodMs) {
        s_lastAcquire = now;
        acquireSample(s_sample, readTempCb, readAdcCb);

        Serial.print(F("[INFO] temp="));
        if (s_sample.sensorError) {
            Serial.print(F("ERR"));
        } else {
            Serial.print(s_sample.temperature);
            Serial.print(F(" °C"));
        }
        Serial.print(F(" adc="));
        Serial.println(s_sample.adc);
    }

    // Servidor HTTP e watchdog (NFR-003)
    s_web.handleClient();
    ESP.wdtFeed();
}
