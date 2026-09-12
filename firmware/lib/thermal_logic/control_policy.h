// control_policy.h — FSM de segurança do controle térmico (lógica pura, sem Arduino).
//
// Responsabilidades: corte em >= 80,00 °C, latch de segurança, rearme manual,
// bloqueio da carga sem leitura válida e recusa explícita de comandos.
// Contrato e ordem de avaliação: .specs/features/controle-termico/design.md §3.1 (ADR-003).
// Restrição de tempo real: nenhum método bloqueia; tudo é síncrono e determinístico.
#pragma once

#include <stdint.h>

namespace thermal {

// Motivo observável de bloqueio/recusa (telemetria /json e respostas HTTP).
enum class Reason : uint8_t {
  kNone = 0,
  kNoSensor,        // nenhum DS18B20 detectado no barramento
  kInvalidReading,  // sensor presente, leitura inválida ou erro de comunicação
  kLatched,         // latch de segurança ativo (corte por >= 80,0 °C)
  kNotLatched,      // rearme pedido sem latch ativo
  kTempHigh,        // rearme recusado: leitura válida >= 80,0 °C
};

// Nome estável do motivo (contrato de telemetria; nome de código, não localizado).
const char* reasonName(Reason r);

// Fatos de entrada de cada avaliação (vêm da amostragem, nunca de comandos).
struct PolicyInputs {
  bool sensor_present;  // resultado da varredura OneWire
  bool reading_valid;   // validade da última leitura do DS18B20
  int16_t temp_centi;   // centésimos de °C (usado apenas quando reading_valid)
};

// Resultado observável de um comando (para feedback ao operador).
struct CommandOutcome {
  bool accepted;   // comando efetivado?
  Reason reason;   // motivo da recusa quando accepted == false
  uint16_t pwm;    // PWM resultante (sempre observável)
  bool latch;      // latch após o comando
};

// FSM de segurança. O PWM é SEMPRE derivado do estado interno (nunca comandado
// diretamente), o que garante que nenhum caminho esqueça a carga ligada sob bloqueio.
class ControlPolicy {
 public:
  static constexpr int16_t kSafetyTempCenti = 8000;  // fronteira inclusiva: >= 80,00 °C
  static constexpr uint16_t kPwmMax = 1023;          // 10 bits (analogWriteRange(1023))

  // Estado inicial seguro: carga desligada, latch limpo e bloqueio até a primeira
  // leitura válida (DEC-02 — vale para boot/reset/watchdog/brownout).
  void begin();

  // Reavalia a segurança com os fatos da amostra atual (ordem fixa — ADR-003):
  // 1) alarme (somente leitura válida) → 2) latch → 3) bloqueio por prioridade →
  // 4) fail-safe: bloqueado limpa o comando de ligar.
  void update(const PolicyInputs& in);

  // Comandos avaliados contra o último update() (dashboard/HTTP/console — DEC-01).
  CommandOutcome requestHeater(bool on);  // ON recusado sob bloqueio; OFF sempre aceito
  CommandOutcome requestRearm();          // aceite só com leitura válida < 80,00 °C

  // Observáveis para atuação e telemetria.
  uint16_t pwm() const { return (heater_on_ && !blocked()) ? kPwmMax : 0; }
  bool alarm() const { return alarm_; }
  bool latch() const { return latch_; }
  bool blocked() const { return block_ != Reason::kNone; }
  Reason block_reason() const { return block_; }
  bool heater_command() const { return heater_on_; }
  const PolicyInputs& inputs() const { return inputs_; }

  // Estado de interface: "NO_SENSOR" | "INVALID_READING" | "NORMAL" | "LATCHED".
  // O latch tem precedência de apresentação; sensor/validade seguem visíveis
  // como campos independentes no /json.
  const char* stateName() const;

 private:
  PolicyInputs inputs_ = {false, false, 0};
  bool heater_on_ = false;  // intenção explícita do operador (limpa ao bloquear)
  bool alarm_ = false;      // >= 80,0 °C com leitura válida (condição, não latch)
  bool latch_ = false;      // trava de segurança volátil (só sai por rearme manual)
  Reason block_ = Reason::kNoSensor;
};

}  // namespace thermal
