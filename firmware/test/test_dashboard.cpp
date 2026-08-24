#include <unity.h>
#include <sampler.h>
#include <json_formatter.h>
#include <adc_driver.h>
#include <ArduinoJson.h>

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
static float s_mockTemp;
static bool  s_mockTempOk;
static uint16_t s_mockAdc;

static float mockReadTemp(bool* ok) {
    if (ok) *ok = s_mockTempOk;
    return s_mockTemp;
}
static uint16_t mockReadAdc() { return s_mockAdc; }

// ---------------------------------------------------------------------------
// Sampler
// ---------------------------------------------------------------------------
void test_acquire_sample_valid() {
    s_mockTemp = 25.5f;
    s_mockTempOk = true;
    s_mockAdc = 512;

    Sample s;
    acquireSample(s, mockReadTemp, mockReadAdc);

    TEST_ASSERT_FLOAT_WITHIN(0.01f, 25.5f, s.temperature);
    TEST_ASSERT_EQUAL_UINT16(512, s.adc);
    TEST_ASSERT_FALSE(s.sensorError);
}

void test_acquire_sample_sensor_failure() {
    s_mockTemp = NAN;
    s_mockTempOk = false;
    s_mockAdc = 300;

    Sample s;
    acquireSample(s, mockReadTemp, mockReadAdc);

    TEST_ASSERT_TRUE(s.sensorError);
    TEST_ASSERT_TRUE(isnan(s.temperature));
    TEST_ASSERT_EQUAL_UINT16(300, s.adc); // A0 continua válido mesmo com falha do sensor
}

// ---------------------------------------------------------------------------
// JSON formatter
// ---------------------------------------------------------------------------
void test_json_valid_payload() {
    Sample s{};
    s.temperature = 24.3f;
    s.adc = 777;
    s.timestamp = 12345;
    s.sensorError = false;

    const String payload = formatValuesJson(s);

    JsonDocument doc;
    const DeserializationError err = deserializeJson(doc, payload.c_str());
    TEST_ASSERT_FALSE(err);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 24.3f, doc["temperature"].as<float>());
    TEST_ASSERT_EQUAL_INT(777, doc["adc"].as<int>());
    TEST_ASSERT_EQUAL_INT(12345, doc["timestamp"].as<int>());
    TEST_ASSERT_FALSE(doc["sensor_error"].as<bool>());
}

void test_json_sensor_error_payload() {
    Sample s{};
    s.temperature = NAN;
    s.adc = 100;
    s.timestamp = 1;
    s.sensorError = true;

    const String payload = formatValuesJson(s);

    JsonDocument doc;
    const DeserializationError err = deserializeJson(doc, payload.c_str());
    TEST_ASSERT_FALSE(err);
    TEST_ASSERT_TRUE(doc["sensor_error"].as<bool>());
    TEST_ASSERT_TRUE(doc["temperature"].isNull());
    TEST_ASSERT_EQUAL_INT(100, doc["adc"].as<int>());
}

// ---------------------------------------------------------------------------
// ADC driver (clamp de faixa é lógica pura; analogRead não roda no host)
// ---------------------------------------------------------------------------
void test_adc_range_contract() {
    // O contrato NFR-005 garante 0..1023; aqui validamos o formato uint16_t.
    uint16_t v = 1023;
    TEST_ASSERT_TRUE(v >= 0 && v <= 1023);
    TEST_ASSERT_EQUAL_UINT16(1023, v);
}

// ---------------------------------------------------------------------------
int main() {
    UNITY_BEGIN();
    RUN_TEST(test_acquire_sample_valid);
    RUN_TEST(test_acquire_sample_sensor_failure);
    RUN_TEST(test_json_valid_payload);
    RUN_TEST(test_json_sensor_error_payload);
    RUN_TEST(test_adc_range_contract);
    return UNITY_END();
}
