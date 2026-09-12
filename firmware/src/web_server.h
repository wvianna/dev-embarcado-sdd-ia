// web_server.h — servidor HTTP: dashboard, telemetria e comandos (contrato design §4.5).
//
// O módulo não conhece a política diretamente: recebe hooks do main (composição),
// o que mantém a FSM como owner único do estado.
#pragma once

#include <stddef.h>
#include <stdint.h>

namespace web {

// Resultado observável de um comando (para o feedback ao operador — FR-009/FR-012).
struct CommandResult {
  bool ok;             // comando efetivado?
  const char* reason;  // motivo estável da recusa ("none" quando aceito)
  uint16_t pwm;        // PWM resultante (0–1023)
  bool latch;          // latch após o comando
  const char* state;   // estado de interface (NO_SENSOR/INVALID_READING/NORMAL/LATCHED)
};

struct Handlers {
  CommandResult (*heater)(bool on);        // ON/OFF (mesma política do console)
  CommandResult (*rearm)();                // rearme manual do latch
  size_t (*fill_status)(char* out, size_t cap);  // preenche o payload /json
};

void begin(const Handlers& hooks);
void poll();  // atende clientes (não bloqueante, cooperativo)

}  // namespace web
