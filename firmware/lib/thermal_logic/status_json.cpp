// status_json.cpp — implementação do contrato /json (ver status_json.h).
#include "status_json.h"

#include <stdarg.h>
#include <stdio.h>

namespace thermal {
namespace {

// Reserva de cauda suficiente para `,"hist_count":NNNN,"hist":[` + `]}` + NUL.
constexpr size_t kTailReserve = 40;

// Acumula texto com limite rígido: nunca escreve além de `cap`; em truncamento,
// satura em cap-1 (sempre NUL-terminado dentro da capacidade).
size_t appendf(char* out, size_t cap, size_t n, const char* fmt, ...) {
  if (n + 1 >= cap) {
    return (cap > 0) ? cap - 1 : 0;
  }
  va_list args;
  va_start(args, fmt);
  const int written = vsnprintf(out + n, cap - n, fmt, args);
  va_end(args);
  if (written < 0) {
    return n;
  }
  if (static_cast<size_t>(written) >= cap - n) {
    return cap - 1;  // vsnprintf truncou: satura no último byte útil
  }
  return n + static_cast<size_t>(written);
}

size_t intDigits(int value) {
  size_t digits = 1;
  if (value < 0) {
    ++digits;
    value = -value;
  }
  while (value >= 10) {
    value /= 10;
    ++digits;
  }
  return digits;
}

}  // namespace

size_t buildStatusJson(char* out, size_t cap, const StatusSnapshot& s) {
  if (cap == 0) {
    return 0;
  }
  out[0] = '\0';

  const uint8_t load = (s.load_pct > 100) ? 100 : s.load_pct;
  const uint8_t idle = static_cast<uint8_t>(100 - load);

  size_t n = 0;
  n = appendf(out, cap, n,
              "{\"fw\":\"%s\",\"uptime_ms\":%lu,\"state\":\"%s\",\"sensor\":%s,"
              "\"valid\":%s,\"temp\":%d,\"pwm\":%u,\"latch\":%s,\"alarm\":%s,"
              "\"block\":\"%s\",\"load_pct\":%u,\"idle_pct\":%u,\"ram_free\":%lu,"
              "\"heap_frag\":%u,\"flash_used\":%lu,\"flash_pct\":%u,\"samp_ms\":%lu,"
              "\"samp_min_ms\":%lu,\"samp_max_ms\":%lu,\"samp_win_min_ms\":%lu,"
              "\"samp_win_max_ms\":%lu,\"ssid\":\"%s\",\"ip\":\"%s\","
              "\"clients\":%u,\"reset\":\"%s\",\"hist_period_ms\":%lu",
              s.fw, static_cast<unsigned long>(s.uptime_ms), s.state,
              s.sensor ? "true" : "false", s.valid ? "true" : "false",
              static_cast<int>(s.temp_centi), static_cast<unsigned>(s.pwm),
              s.latch ? "true" : "false", s.alarm ? "true" : "false", s.block,
              static_cast<unsigned>(load), static_cast<unsigned>(idle),
              static_cast<unsigned long>(s.ram_free), static_cast<unsigned>(s.heap_frag),
              static_cast<unsigned long>(s.flash_used), static_cast<unsigned>(s.flash_pct),
              static_cast<unsigned long>(s.samp_ms),
              static_cast<unsigned long>(s.samp_min_ms),
              static_cast<unsigned long>(s.samp_max_ms),
              static_cast<unsigned long>(s.samp_win_min_ms),
              static_cast<unsigned long>(s.samp_win_max_ms), s.ssid, s.ip,
              static_cast<unsigned>(s.clients), s.reset_reason,
              static_cast<unsigned long>(s.hist_period_ms));

  // Quantas amostras do histórico cabem exatamente (custo dígito a dígito)?
  size_t fitted = 0;
  if (s.hist != nullptr && s.hist_count > 0 && cap > kTailReserve + 2) {
    const size_t limit = cap - 1 - kTailReserve;  // -1: NUL final
    size_t used = 0;
    for (size_t i = 0; i < s.hist_count; ++i) {
      const size_t cost = intDigits(static_cast<int>(s.hist[i])) + (i == 0 ? 0 : 1);
      if (n + used + cost > limit) {
        break;
      }
      used += cost;
      ++fitted;
    }
  }

  n = appendf(out, cap, n, ",\"hist_count\":%u,\"hist\":[", static_cast<unsigned>(fitted));
  for (size_t i = 0; i < fitted; ++i) {
    n = appendf(out, cap, n, (i == 0) ? "%d" : ",%d", static_cast<int>(s.hist[i]));
  }
  n = appendf(out, cap, n, "]}");

  if (n >= cap) {
    n = cap - 1;  // segurança extra em caso de saturação
  }
  out[n] = '\0';
  return n;
}

}  // namespace thermal
