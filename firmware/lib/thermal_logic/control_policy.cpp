// control_policy.cpp — implementação da FSM de segurança (ver control_policy.h).
#include "control_policy.h"

namespace thermal {

const char* reasonName(Reason r) {
  switch (r) {
    case Reason::kNoSensor:
      return "no_sensor";
    case Reason::kInvalidReading:
      return "invalid_reading";
    case Reason::kLatched:
      return "latched";
    case Reason::kNotLatched:
      return "not_latched";
    case Reason::kTempHigh:
      return "temp_high";
    case Reason::kNone:
    default:
      return "none";
  }
}

void ControlPolicy::begin() {
  inputs_ = {false, false, 0};
  heater_on_ = false;
  alarm_ = false;
  latch_ = false;
  block_ = Reason::kNoSensor;  // ainda sem leitura válida: carga bloqueada
}

void ControlPolicy::update(const PolicyInputs& in) {
  inputs_ = in;

  // 1) Alarme: apenas leitura válida conta; >= 80,00 °C é condição de segurança
  //    (não é leitura inválida — FR-006).
  alarm_ = in.sensor_present && in.reading_valid && in.temp_centi >= kSafetyTempCenti;

  // 2) Corte na MESMA avaliação + latch (FR-010): vale mesmo com a carga ligada.
  if (alarm_) {
    latch_ = true;
  }

  // 3) Prioridade de bloqueio: latch > sensor ausente > leitura inválida.
  if (latch_) {
    block_ = Reason::kLatched;
  } else if (!in.sensor_present) {
    block_ = Reason::kNoSensor;
  } else if (!in.reading_valid) {
    block_ = Reason::kInvalidReading;
  } else {
    block_ = Reason::kNone;
  }

  // 4) Fail-safe: nenhum comando de ligar sobrevive a um bloqueio; ao desbloquear,
  //    a carga permanece desligada até novo ON explícito (DEC-04).
  if (blocked()) {
    heater_on_ = false;
  }
}

CommandOutcome ControlPolicy::requestHeater(bool on) {
  if (!on) {
    heater_on_ = false;  // OFF é sempre aceito e sempre seguro
    return {true, Reason::kNone, pwm(), latch_};
  }
  if (blocked()) {
    return {false, block_, pwm(), latch_};  // recusa explícita com motivo (FR-009)
  }
  heater_on_ = true;
  return {true, Reason::kNone, pwm(), latch_};
}

CommandOutcome ControlPolicy::requestRearm() {
  if (!latch_) {
    return {false, Reason::kNotLatched, pwm(), latch_};
  }
  if (!inputs_.sensor_present) {
    return {false, Reason::kNoSensor, pwm(), latch_};
  }
  if (!inputs_.reading_valid) {
    return {false, Reason::kInvalidReading, pwm(), latch_};
  }
  if (inputs_.temp_centi >= kSafetyTempCenti) {
    return {false, Reason::kTempHigh, pwm(), latch_};  // ainda quente: mantém o latch
  }

  latch_ = false;
  block_ = Reason::kNone;
  heater_on_ = false;  // após o rearme a carga permanece desligada até novo ON (DEC-01)
  return {true, Reason::kNone, pwm(), latch_};
}

const char* ControlPolicy::stateName() const {
  if (latch_) {
    return "LATCHED";
  }
  if (!inputs_.sensor_present) {
    return "NO_SENSOR";
  }
  if (!inputs_.reading_valid) {
    return "INVALID_READING";
  }
  return "NORMAL";
}

}  // namespace thermal
