// Testes HOST do padrão do buzzer (env native).
// FR-013: 150 ms ON / 2000 ms OFF apenas enquanto a condição persistir.
#include <unity.h>

#include "buzzer_pattern.h"

using thermal::BuzzerPattern;

// Constantes do contrato de alarme (não podem derivar silenciosamente).
static_assert(BuzzerPattern::kOnMs == 150, "fase ON deve ser 150 ms");
static_assert(BuzzerPattern::kOffMs == 2000, "fase OFF deve ser 2000 ms");

namespace {
BuzzerPattern buzzer;
}

void setUp() { buzzer.begin(0); }
void tearDown() {}

void test_inactive_without_condition() {
  TEST_ASSERT_FALSE(buzzer.update(0, false));
  TEST_ASSERT_FALSE(buzzer.update(5000, false));
}

void test_activation_starts_in_on_phase() {
  TEST_ASSERT_TRUE(buzzer.update(100, true));
  TEST_ASSERT_TRUE(buzzer.update(249, true));   // 149 ms após ativação
  TEST_ASSERT_FALSE(buzzer.update(250, true));  // 150 ms: entra no OFF
}

void test_cycle_boundaries() {
  TEST_ASSERT_TRUE(buzzer.update(0, true));
  TEST_ASSERT_TRUE(buzzer.update(149, true));
  TEST_ASSERT_FALSE(buzzer.update(150, true));
  TEST_ASSERT_FALSE(buzzer.update(2149, true));
  TEST_ASSERT_TRUE(buzzer.update(2150, true));  // novo ciclo começa em ON
  TEST_ASSERT_TRUE(buzzer.update(2299, true));
  TEST_ASSERT_FALSE(buzzer.update(2300, true));
}

void test_condition_clears_disables_immediately() {
  TEST_ASSERT_TRUE(buzzer.update(0, true));
  TEST_ASSERT_FALSE(buzzer.update(50, false));    // alarme cessou: OFF imediato
  TEST_ASSERT_FALSE(buzzer.update(2160, false));  // permanece OFF sem condição
}

void test_reactivation_restarts_on_phase() {
  TEST_ASSERT_TRUE(buzzer.update(0, true));
  TEST_ASSERT_FALSE(buzzer.update(500, false));  // meio do OFF
  TEST_ASSERT_TRUE(buzzer.update(600, true));    // religa começando em ON
  TEST_ASSERT_TRUE(buzzer.update(749, true));
  TEST_ASSERT_FALSE(buzzer.update(750, true));   // 150 ms do novo ciclo
}

void test_wraparound_of_millis_is_safe() {
  TEST_ASSERT_TRUE(buzzer.update(0xFFFFFFF0u, true));
  TEST_ASSERT_TRUE(buzzer.update(0xFFFFFFF0u, true));  // t0: ON
  TEST_ASSERT_FALSE(buzzer.update(0x000000B8u, true)); // +200 ms: OFF (wrap)
  TEST_ASSERT_TRUE(buzzer.update(0x00000856u, true));  // +2150 ms: ON (novo ciclo)
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_inactive_without_condition);
  RUN_TEST(test_activation_starts_in_on_phase);
  RUN_TEST(test_cycle_boundaries);
  RUN_TEST(test_condition_clears_disables_immediately);
  RUN_TEST(test_reactivation_restarts_on_phase);
  RUN_TEST(test_wraparound_of_millis_is_safe);
  return UNITY_END();
}
