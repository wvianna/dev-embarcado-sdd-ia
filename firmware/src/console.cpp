// console.cpp — console de bancada: injeção, comandos e leitura de estado (FR-030).
//
// Compilado apenas no env `bancada` (ADR-007). A injeção substitui a fonte de
// leitura na amostragem; a política, o buzzer, o histórico e o JSON seguem os
// mesmos caminhos do sistema real.
#include "console.h"

#if defined(BENCH_TEMP_INJECTION)

#include <Arduino.h>
#include <stdlib.h>
#include <strings.h>

#include "config.h"

namespace console {
namespace {

Hooks g_hooks = {nullptr, nullptr, nullptr, nullptr};

char g_line[64];
size_t g_len = 0;

// Estado da injeção (ativo até o comando REAL).
bool g_inject_active = false;
bool g_inject_valid = false;
int16_t g_inject_centi = 0;

void printState() {
  static char out[cfg::kJsonBufSize];
  g_hooks.fill_status(out, sizeof(out));
  Serial.printf("JSON %s\n", out);
}

void handleCommand(char* line) {
  const char* cmd = strtok(line, " \t");
  if (cmd == nullptr) {
    return;
  }
  const char* arg = strtok(nullptr, " \t");

  if (strcasecmp(cmd, "TEMP") == 0 && arg != nullptr) {
    g_inject_active = true;
    g_inject_valid = true;
    g_inject_centi = static_cast<int16_t>(atoi(arg));
    Serial.printf("OK TEMP %d\n", static_cast<int>(g_inject_centi));
  } else if (strcasecmp(cmd, "FAULT") == 0) {
    g_inject_active = true;
    g_inject_valid = false;
    Serial.println("OK FAULT");
  } else if (strcasecmp(cmd, "REAL") == 0) {
    g_inject_active = false;
    g_inject_valid = false;
    Serial.println("OK REAL");
  } else if (strcasecmp(cmd, "ON") == 0 || strcasecmp(cmd, "OFF") == 0) {
    const bool on = (strcasecmp(cmd, "ON") == 0);
    const web::CommandResult r = g_hooks.heater(on);
    if (r.ok) {
      Serial.printf("OK %s pwm=%u\n", on ? "ON" : "OFF", static_cast<unsigned>(r.pwm));
    } else {
      Serial.printf("ERR %s reason=%s\n", on ? "ON" : "OFF", r.reason);
    }
  } else if (strcasecmp(cmd, "REARM") == 0) {
    const web::CommandResult r = g_hooks.rearm();
    if (r.ok) {
      Serial.println("OK REARM");
    } else {
      Serial.printf("ERR REARM reason=%s\n", r.reason);
    }
  } else if (strcasecmp(cmd, "STATE") == 0) {
    printState();
  } else if (strcasecmp(cmd, "RESCAN") == 0) {
    const bool present = g_hooks.rescan();
    Serial.printf("OK RESCAN sensor=%d\n", present ? 1 : 0);
  } else if (strcasecmp(cmd, "REBOOT") == 0) {
    Serial.println("OK REBOOT");
    Serial.flush();
    ESP.restart();
  } else if (strcasecmp(cmd, "HELP") == 0) {
    Serial.println("OK HELP TEMP <centi> | FAULT | REAL | ON | OFF | REARM | STATE | RESCAN | REBOOT");
  } else {
    Serial.printf("ERR unknown cmd=%s\n", cmd);
  }
}

}  // namespace

void begin(const Hooks& hooks) {
  g_hooks = hooks;
  g_len = 0;
  Serial.println("[bench] console de injecao ativo — envie HELP para comandos");
}

void poll() {
  while (Serial.available() > 0) {
    const char c = static_cast<char>(Serial.read());
    if (c == '\n' || c == '\r') {
      if (g_len > 0) {
        g_line[g_len] = '\0';
        handleCommand(g_line);
        g_len = 0;
      }
    } else if (g_len < sizeof(g_line) - 1) {
      g_line[g_len++] = c;
    } else {
      g_len = 0;  // linha longa demais: descarta
    }
  }
}

bool injectionActive() { return g_inject_active; }
bool injectionValid() { return g_inject_valid; }
int16_t injectionCenti() { return g_inject_centi; }

}  // namespace console

#endif  // BENCH_TEMP_INJECTION
