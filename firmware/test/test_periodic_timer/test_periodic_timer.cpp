// Testes HOST do agendador não bloqueante (env native).
// Cobre cadência sem drift, consumos repetidos no mesmo ms, reancoragem em
// atraso e overflow de millis() (wrap).
#include <unity.h>

#include "periodic_timer.h"

using thermal::PeriodicTimer;

namespace {
PeriodicTimer timer;
}

void setUp() { timer.begin(0, 1000); }
void tearDown() {}

void test_not_due_before_period() {
  TEST_ASSERT_FALSE(timer.due(999));
  TEST_ASSERT_FALSE(timer.consume(999));
  TEST_ASSERT_FALSE(timer.due(0));
}

void test_due_exactly_at_period() {
  TEST_ASSERT_TRUE(timer.due(1000));
  TEST_ASSERT_TRUE(timer.consume(1000));
  TEST_ASSERT_FALSE(timer.consume(1000));  // não repete no mesmo instante
  TEST_ASSERT_FALSE(timer.due(1999));
  TEST_ASSERT_TRUE(timer.due(2000));
}

void test_cadence_without_drift_over_many_periods() {
  int exact_hits = 0;
  for (uint32_t ms = 0; ms <= 60000; ms += 10) {  // passo exato no deadline
    if (timer.consume(ms)) {
      ++exact_hits;
    }
  }
  TEST_ASSERT_EQUAL_INT(60, exact_hits);  // 60 s => 60 amostras

  timer.begin(0, 1000);
  int irregular_hits = 0;
  for (uint32_t ms = 0; ms <= 60000; ms += 7) {  // passo irregular simula o loop
    if (timer.consume(ms)) {
      ++irregular_hits;
    }
  }
  TEST_ASSERT_INT_WITHIN(1, 60, irregular_hits);  // sem deriva acumulada
}

void test_late_tick_reanchors_without_burst() {
  TEST_ASSERT_TRUE(timer.consume(1000));
  TEST_ASSERT_TRUE(timer.consume(2600));   // vencimento de 2000 atendido com atraso
  TEST_ASSERT_FALSE(timer.consume(2600));  // sem segundo disparo no mesmo instante
  TEST_ASSERT_FALSE(timer.due(2999));      // cadência nominal restaurada
  TEST_ASSERT_TRUE(timer.due(3000));

  TEST_ASSERT_TRUE(timer.consume(3000));
  TEST_ASSERT_TRUE(timer.consume(60000));  // atraso de 56 s (loop preso): reancora
  TEST_ASSERT_FALSE(timer.due(60999));
  TEST_ASSERT_TRUE(timer.due(61000));  // now + período, sem rajada
}

void test_wraparound_of_millis_is_safe() {
  timer.begin(0xFFFFFE00u, 1000);              // deadline = 0x1E8 (com wrap)
  TEST_ASSERT_FALSE(timer.due(0xFFFFFFFFu));   // 489 ms antes do deadline
  TEST_ASSERT_TRUE(timer.due(0x000001E8u));    // deadline alcançado
  TEST_ASSERT_TRUE(timer.consume(0x000001E8u));
  TEST_ASSERT_FALSE(timer.due(0x000005CFu));   // 999 ms após: ainda não
  TEST_ASSERT_TRUE(timer.due(0x000005D0u));    // 1000 ms após: cadência mantida
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_not_due_before_period);
  RUN_TEST(test_due_exactly_at_period);
  RUN_TEST(test_cadence_without_drift_over_many_periods);
  RUN_TEST(test_late_tick_reanchors_without_burst);
  RUN_TEST(test_wraparound_of_millis_is_safe);
  return UNITY_END();
}
