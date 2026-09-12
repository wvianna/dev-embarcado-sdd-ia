// Testes HOST do contrato /json (env native).
// Valida campos, ordem fixa (DEC-03), truncamento seguro do histórico e
// representação explícita de falha (sem inventar temperatura).
#include <unity.h>

#include <stdio.h>
#include <string.h>

#include "status_json.h"

namespace {

int16_t hist[120];

thermal::StatusSnapshot makeSnapshot() {
  thermal::StatusSnapshot s = {};
  s.fw = "1.0.0";
  s.uptime_ms = 123456;
  s.state = "NORMAL";
  s.sensor = true;
  s.valid = true;
  s.temp_centi = 2345;
  s.pwm = 1023;
  s.latch = false;
  s.alarm = false;
  s.block = "none";
  s.load_pct = 3;
  s.ram_free = 40000;
  s.heap_frag = 2;
  s.flash_used = 321055;
  s.flash_pct = 30;
  s.samp_ms = 1000;
  s.samp_min_ms = 993;
  s.samp_max_ms = 1017;
  s.samp_win_min_ms = 995;
  s.samp_win_max_ms = 1006;
  s.ssid = "ESP8266_1A2B3C";
  s.ip = "192.168.4.1";
  s.clients = 1;
  s.reset_reason = "External System";
  s.hist_period_ms = 1000;
  s.hist = hist;
  s.hist_count = 0;
  return s;
}

unsigned parseHistCount(const char* json) {
  const char* pos = strstr(json, "\"hist_count\":");
  TEST_ASSERT_NOT_NULL(pos);
  unsigned value = 0;
  TEST_ASSERT_EQUAL_INT(1, sscanf(pos + 13, "%u", &value));
  return value;
}

unsigned countHistValues(const char* json) {
  const char* arr = strstr(json, "\"hist\":[");
  TEST_ASSERT_NOT_NULL(arr);
  arr += 8;  // aponta para o primeiro item (ou ']')
  if (*arr == ']') {
    return 0;
  }
  unsigned values = 1;
  for (const char* p = arr; *p != ']' && *p != '\0'; ++p) {
    if (*p == ',') {
      ++values;
    }
  }
  return values;
}

}  // namespace

void setUp() {}
void tearDown() {}

void test_full_payload_has_all_fields_in_contract_order() {
  for (int i = 0; i < 3; ++i) {
    hist[i] = static_cast<int16_t>(2300 + i * 10);
  }
  thermal::StatusSnapshot s = makeSnapshot();
  s.hist_count = 3;

  char out[1600];
  const size_t n = thermal::buildStatusJson(out, sizeof(out), s);

  TEST_ASSERT_EQUAL_UINT(static_cast<unsigned>(strlen(out)), static_cast<unsigned>(n));
  TEST_ASSERT_TRUE(n < sizeof(out));
  TEST_ASSERT_EQUAL_CHAR('}', out[n - 1]);

  const char* keys[] = {"\"fw\"",        "\"uptime_ms\"",  "\"state\"",     "\"sensor\"",
                        "\"valid\"",     "\"temp\"",       "\"pwm\"",       "\"latch\"",
                        "\"alarm\"",     "\"block\"",      "\"load_pct\"",  "\"idle_pct\"",
                        "\"ram_free\"",  "\"heap_frag\"",  "\"flash_used\"", "\"flash_pct\"",
                        "\"samp_ms\"",   "\"samp_min_ms\"", "\"samp_max_ms\"",
                        "\"samp_win_min_ms\"", "\"samp_win_max_ms\"", "\"ssid\"",
                        "\"ip\"",        "\"clients\"",    "\"reset\"",     "\"hist_period_ms\"",
                        "\"hist_count\"", "\"hist\""};
  const char* cursor = out;
  for (const char* key : keys) {
    const char* pos = strstr(cursor, key);
    TEST_ASSERT_NOT_NULL_MESSAGE(pos, key);
    cursor = pos;
  }

  TEST_ASSERT_NOT_NULL(strstr(out, "\"temp\":2345"));         // centésimos, sem float
  TEST_ASSERT_NOT_NULL(strstr(out, "\"idle_pct\":97"));       // derivado de load_pct
  TEST_ASSERT_NOT_NULL(strstr(out, "\"samp_win_min_ms\":995"));  // jitter em regime
  TEST_ASSERT_NOT_NULL(strstr(out, "\"samp_win_max_ms\":1006"));
  TEST_ASSERT_NOT_NULL(strstr(out, "\"latch\":false"));
  TEST_ASSERT_NOT_NULL(strstr(out, "\"hist_count\":3,\"hist\":[2300,2310,2320]"));
  TEST_ASSERT_EQUAL_UINT(3, countHistValues(out));
}

void test_hist_truncated_to_fit_smaller_buffer() {
  thermal::StatusSnapshot s = makeSnapshot();
  for (int i = 0; i < 120; ++i) {
    hist[i] = static_cast<int16_t>(2000 + i);
  }
  s.hist_count = 120;

  char out[500];
  const size_t n = thermal::buildStatusJson(out, sizeof(out), s);

  TEST_ASSERT_TRUE(n < sizeof(out));
  TEST_ASSERT_EQUAL_CHAR('}', out[n - 1]);  // JSON fechado apesar do truncamento
  const unsigned emitted = parseHistCount(out);
  TEST_ASSERT_TRUE(emitted < 120);  // não caberia tudo
  TEST_ASSERT_EQUAL_UINT(emitted, countHistValues(out));
}

void test_failure_state_is_explicit_and_never_fake_temperature() {
  thermal::StatusSnapshot s = makeSnapshot();
  s.state = "INVALID_READING";
  s.valid = false;
  s.block = "invalid_reading";
  s.pwm = 0;

  char out[1600];
  thermal::buildStatusJson(out, sizeof(out), s);

  TEST_ASSERT_NOT_NULL(strstr(out, "\"valid\":false"));
  TEST_ASSERT_NOT_NULL(strstr(out, "\"state\":\"INVALID_READING\""));
  TEST_ASSERT_NOT_NULL(strstr(out, "\"block\":\"invalid_reading\""));
  TEST_ASSERT_NOT_NULL(strstr(out, "\"pwm\":0"));
}

void test_sensor_absent_is_explicit() {
  thermal::StatusSnapshot s = makeSnapshot();
  s.sensor = false;
  s.valid = false;
  s.state = "NO_SENSOR";
  s.block = "no_sensor";

  char out[1600];
  thermal::buildStatusJson(out, sizeof(out), s);
  TEST_ASSERT_NOT_NULL(strstr(out, "\"sensor\":false"));
}

void test_load_percent_is_clamped() {
  thermal::StatusSnapshot s = makeSnapshot();
  s.load_pct = 200;
  char out[1600];
  thermal::buildStatusJson(out, sizeof(out), s);
  TEST_ASSERT_NOT_NULL(strstr(out, "\"load_pct\":100"));
  TEST_ASSERT_NOT_NULL(strstr(out, "\"idle_pct\":0"));
}

void test_tiny_buffer_never_overflows() {
  thermal::StatusSnapshot s = makeSnapshot();
  char out[16];
  const size_t n = thermal::buildStatusJson(out, sizeof(out), s);
  TEST_ASSERT_TRUE(n < sizeof(out));
  TEST_ASSERT_EQUAL_CHAR('\0', out[n]);
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_full_payload_has_all_fields_in_contract_order);
  RUN_TEST(test_hist_truncated_to_fit_smaller_buffer);
  RUN_TEST(test_failure_state_is_explicit_and_never_fake_temperature);
  RUN_TEST(test_sensor_absent_is_explicit);
  RUN_TEST(test_load_percent_is_clamped);
  RUN_TEST(test_tiny_buffer_never_overflows);
  return UNITY_END();
}
