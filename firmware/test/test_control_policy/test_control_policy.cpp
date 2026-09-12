// Testes HOST da FSM de segurança (env native — sem hardware).
// Cobertura das fronteiras críticas: 79,99/80,00 °C, latch, rearme, bloqueios,
// fail-safe (nada religa sozinho) e recuperação de falha de sensor.
#include <unity.h>

#include "control_policy.h"

using thermal::CommandOutcome;
using thermal::ControlPolicy;
using thermal::PolicyInputs;
using thermal::Reason;

// Constantes de segurança não podem derivar silenciosamente (contrato da spec).
static_assert(ControlPolicy::kSafetyTempCenti == 8000, "fronteira deve ser 80,00 C");
static_assert(ControlPolicy::kPwmMax == 1023, "PWM deve ser de 10 bits");

namespace {

ControlPolicy policy;

PolicyInputs facts(bool present, bool valid, int16_t centi) {
  return PolicyInputs{present, valid, centi};
}

bool reasonIs(CommandOutcome out, Reason expected) { return out.reason == expected; }

}  // namespace

void setUp() { policy.begin(); }
void tearDown() {}

void test_boot_blocks_until_valid_reading() {
  TEST_ASSERT_TRUE(policy.blocked());
  TEST_ASSERT_EQUAL_UINT16(0, policy.pwm());
  TEST_ASSERT_FALSE(policy.latch());
  TEST_ASSERT_FALSE(policy.alarm());
  TEST_ASSERT_EQUAL_STRING("NO_SENSOR", policy.stateName());
  const CommandOutcome on = policy.requestHeater(true);
  TEST_ASSERT_FALSE(on.accepted);
  TEST_ASSERT_TRUE(reasonIs(on, Reason::kNoSensor));
  TEST_ASSERT_EQUAL_UINT16(0, on.pwm);
}

void test_sensor_present_without_readings_blocks_invalid() {
  policy.update(facts(true, false, 0));
  TEST_ASSERT_EQUAL_STRING("INVALID_READING", policy.stateName());
  TEST_ASSERT_TRUE(policy.blocked());
  const CommandOutcome on = policy.requestHeater(true);
  TEST_ASSERT_FALSE(on.accepted);
  TEST_ASSERT_TRUE(reasonIs(on, Reason::kInvalidReading));
}

void test_on_off_with_valid_reading() {
  policy.update(facts(true, true, 2500));
  TEST_ASSERT_FALSE(policy.blocked());
  TEST_ASSERT_EQUAL_STRING("NORMAL", policy.stateName());

  const CommandOutcome on = policy.requestHeater(true);
  TEST_ASSERT_TRUE(on.accepted);
  TEST_ASSERT_EQUAL_UINT16(1023, on.pwm);
  TEST_ASSERT_EQUAL_UINT16(1023, policy.pwm());

  const CommandOutcome off = policy.requestHeater(false);
  TEST_ASSERT_TRUE(off.accepted);
  TEST_ASSERT_EQUAL_UINT16(0, off.pwm);
}

void test_boundary_below_keeps_heater_on() {
  policy.update(facts(true, true, 2500));
  policy.requestHeater(true);
  policy.update(facts(true, true, 7999));  // 79,99 °C: ainda abaixo da fronteira
  TEST_ASSERT_FALSE(policy.alarm());
  TEST_ASSERT_FALSE(policy.latch());
  TEST_ASSERT_EQUAL_UINT16(1023, policy.pwm());
}

void test_boundary_at_8000_cuts_and_latches_same_evaluation() {
  policy.update(facts(true, true, 2500));
  policy.requestHeater(true);
  policy.update(facts(true, true, 8000));  // fronteira inclusiva
  TEST_ASSERT_TRUE(policy.alarm());
  TEST_ASSERT_TRUE(policy.latch());
  TEST_ASSERT_EQUAL_UINT16(0, policy.pwm());  // corte na MESMA avaliação
  TEST_ASSERT_EQUAL_STRING("LATCHED", policy.stateName());

  const CommandOutcome on = policy.requestHeater(true);
  TEST_ASSERT_FALSE(on.accepted);
  TEST_ASSERT_TRUE(reasonIs(on, Reason::kLatched));
  TEST_ASSERT_EQUAL_UINT16(0, on.pwm);
}

void test_latch_persists_after_cooling() {
  policy.update(facts(true, true, 9000));
  policy.update(facts(true, true, 5000));
  TEST_ASSERT_FALSE(policy.alarm());  // condição cessou (buzzer para)
  TEST_ASSERT_TRUE(policy.latch());   // trava permanece
  TEST_ASSERT_TRUE(policy.blocked());
  TEST_ASSERT_EQUAL_UINT16(0, policy.pwm());
}

void test_rearm_requires_valid_cool_reading() {
  policy.update(facts(true, true, 9000));
  const CommandOutcome denied = policy.requestRearm();  // ainda a 90,0 °C
  TEST_ASSERT_FALSE(denied.accepted);
  TEST_ASSERT_TRUE(reasonIs(denied, Reason::kTempHigh));
  TEST_ASSERT_TRUE(policy.latch());

  policy.update(facts(true, true, 5000));  // esfriou com leitura válida
  const CommandOutcome ok = policy.requestRearm();
  TEST_ASSERT_TRUE(ok.accepted);
  TEST_ASSERT_FALSE(policy.latch());
  TEST_ASSERT_EQUAL_UINT16(0, ok.pwm);  // carga continua desligada (DEC-01)

  const CommandOutcome on = policy.requestHeater(true);  // novo ON é obrigatório
  TEST_ASSERT_TRUE(on.accepted);
  TEST_ASSERT_EQUAL_UINT16(1023, on.pwm);
}

void test_rearm_denied_when_not_latched() {
  policy.update(facts(true, true, 2500));
  const CommandOutcome denied = policy.requestRearm();
  TEST_ASSERT_FALSE(denied.accepted);
  TEST_ASSERT_TRUE(reasonIs(denied, Reason::kNotLatched));
  TEST_ASSERT_FALSE(policy.latch());
}

void test_rearm_denied_on_invalid_reading_or_no_sensor() {
  policy.update(facts(true, true, 9000));  // latcheia
  policy.update(facts(true, false, 0));    // falha durante o latch
  const CommandOutcome d1 = policy.requestRearm();
  TEST_ASSERT_FALSE(d1.accepted);
  TEST_ASSERT_TRUE(reasonIs(d1, Reason::kInvalidReading));
  TEST_ASSERT_TRUE(policy.latch());

  policy.update(facts(false, false, 0));  // sensor ausente
  const CommandOutcome d2 = policy.requestRearm();
  TEST_ASSERT_FALSE(d2.accepted);
  TEST_ASSERT_TRUE(reasonIs(d2, Reason::kNoSensor));
  TEST_ASSERT_TRUE(policy.latch());
}

void test_invalid_reading_cuts_and_disables_alarm_condition() {
  policy.update(facts(true, true, 2500));
  policy.requestHeater(true);
  policy.update(facts(true, false, 0));  // erro de comunicação
  TEST_ASSERT_FALSE(policy.alarm());     // buzzer não opera em leitura inválida
  TEST_ASSERT_EQUAL_UINT16(0, policy.pwm());
  TEST_ASSERT_TRUE(policy.block_reason() == Reason::kInvalidReading);

  const CommandOutcome on = policy.requestHeater(true);
  TEST_ASSERT_FALSE(on.accepted);
}

void test_recovery_does_not_auto_restart() {
  policy.update(facts(true, true, 2500));
  policy.requestHeater(true);
  policy.update(facts(true, false, 0));    // bloqueia (corte)
  policy.update(facts(true, true, 2500));  // leitura válida volta (sem latch)
  TEST_ASSERT_FALSE(policy.blocked());
  TEST_ASSERT_EQUAL_UINT16(0, policy.pwm());  // NÃO religou sozinho (DEC-04)
  TEST_ASSERT_FALSE(policy.heater_command());

  const CommandOutcome on = policy.requestHeater(true);
  TEST_ASSERT_TRUE(on.accepted);
  TEST_ASSERT_EQUAL_UINT16(1023, on.pwm);
}

void test_sensor_loss_and_return_does_not_auto_restart() {
  policy.update(facts(true, true, 2500));
  policy.requestHeater(true);
  policy.update(facts(false, false, 0));  // desconexão física
  TEST_ASSERT_TRUE(policy.block_reason() == Reason::kNoSensor);
  TEST_ASSERT_EQUAL_UINT16(0, policy.pwm());

  policy.update(facts(true, true, 2500));  // reconectado
  TEST_ASSERT_FALSE(policy.blocked());
  TEST_ASSERT_EQUAL_UINT16(0, policy.pwm());
}

void test_off_is_always_accepted() {
  policy.update(facts(false, false, 0));
  const CommandOutcome off = policy.requestHeater(false);
  TEST_ASSERT_TRUE(off.accepted);
  TEST_ASSERT_EQUAL_UINT16(0, off.pwm);
}

void test_latch_priority_over_sensor_failure() {
  policy.update(facts(true, true, 9000));  // latch
  policy.update(facts(false, false, 0));   // sensor ausente com latch ativo
  TEST_ASSERT_TRUE(policy.block_reason() == Reason::kLatched);
  TEST_ASSERT_EQUAL_STRING("LATCHED", policy.stateName());
}

void test_latch_is_volatile_across_begin() {
  policy.update(facts(true, true, 9000));
  policy.begin();  // simula reset/watchdog: estado seguro volátil
  TEST_ASSERT_FALSE(policy.latch());
  TEST_ASSERT_TRUE(policy.blocked());
  TEST_ASSERT_EQUAL_UINT16(0, policy.pwm());
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_boot_blocks_until_valid_reading);
  RUN_TEST(test_sensor_present_without_readings_blocks_invalid);
  RUN_TEST(test_on_off_with_valid_reading);
  RUN_TEST(test_boundary_below_keeps_heater_on);
  RUN_TEST(test_boundary_at_8000_cuts_and_latches_same_evaluation);
  RUN_TEST(test_latch_persists_after_cooling);
  RUN_TEST(test_rearm_requires_valid_cool_reading);
  RUN_TEST(test_rearm_denied_when_not_latched);
  RUN_TEST(test_rearm_denied_on_invalid_reading_or_no_sensor);
  RUN_TEST(test_invalid_reading_cuts_and_disables_alarm_condition);
  RUN_TEST(test_recovery_does_not_auto_restart);
  RUN_TEST(test_sensor_loss_and_return_does_not_auto_restart);
  RUN_TEST(test_off_is_always_accepted);
  RUN_TEST(test_latch_priority_over_sensor_failure);
  RUN_TEST(test_latch_is_volatile_across_begin);
  return UNITY_END();
}
