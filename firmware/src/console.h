// console.h — console de diagnóstico de bancada (somente env `bancada`, FR-030).
//
// O módulo inteiro existe apenas quando BENCH_TEMP_INJECTION está definido
// (ADR-007): o binário de produção não contém os comandos nem a injeção.
// Protocolo (design §4.6): linhas ASCII; resposta em uma linha `OK …`/`ERR …`/`JSON …`.
#pragma once

#if defined(BENCH_TEMP_INJECTION)

#include <stddef.h>
#include <stdint.h>

#include "web_server.h"

namespace console {

struct Hooks {
  web::CommandResult (*heater)(bool on);
  web::CommandResult (*rearm)();
  size_t (*fill_status)(char* out, size_t cap);
  bool (*rescan)();
};

void begin(const Hooks& hooks);
void poll();  // leitura serial não bloqueante

// Injeção de temperatura/falha (substitui a amostragem real até `REAL`).
bool injectionActive();
bool injectionValid();
int16_t injectionCenti();

}  // namespace console

#endif  // BENCH_TEMP_INJECTION
