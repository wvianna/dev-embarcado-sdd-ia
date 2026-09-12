// status_json.h — serialização do contrato /json em buffer estático (lógica pura).
//
// Contrato fixado em design §3.5 (DEC-03): ordem de campos estável, valores
// inteiros (sem float), temperatura em centésimos de °C. O builder NUNCA
// ultrapassa `cap`: o histórico é truncado ao que couber e `hist_count` reflete
// exatamente o emitido. Garantia de JSON válido a partir do tamanho mínimo do
// prefixo (~350 B); abaixo disso o truncamento segue seguro (sem overrun, com
// NUL em `out[cap-1]`).
#pragma once

#include <stddef.h>
#include <stdint.h>

namespace thermal {

// Snapshot imutável montado pelo chamador (main/console) no momento da serialização.
struct StatusSnapshot {
  const char* fw;           // versão do firmware (config.h)
  uint32_t uptime_ms;       // millis()
  const char* state;        // ControlPolicy::stateName()
  bool sensor;              // presença do DS18B20
  bool valid;               // validade da última leitura
  int16_t temp_centi;       // centésimos de °C (congelado quando inválida)
  uint16_t pwm;             // PWM aplicado à carga (0–1023)
  bool latch;               // latch de segurança
  bool alarm;               // >= 80,0 °C com leitura válida
  const char* block;        // reasonName() do bloqueio
  uint8_t load_pct;         // ocupação da janela de 1 s (design §5.1)
  uint32_t ram_free;        // ESP.getFreeHeap()
  uint8_t heap_frag;        // ESP.getHeapFragmentation()
  uint32_t flash_used;      // ESP.getSketchSize()
  uint8_t flash_pct;        // flash_used * 100 / 1044464
  uint32_t samp_ms;         // último intervalo de amostragem
  uint32_t samp_min_ms;     // mínimo desde o boot (evidência CA-004)
  uint32_t samp_max_ms;     // máximo desde o boot
  uint32_t samp_win_min_ms; // mínimo dos últimos ≤60 intervalos (regime)
  uint32_t samp_win_max_ms; // máximo dos últimos ≤60 intervalos (regime)
  const char* ssid;         // SSID do AP
  const char* ip;           // IP do AP
  uint8_t clients;          // clientes associados
  const char* reset_reason; // ESP.getResetReason()
  uint32_t hist_period_ms;  // período nominal do histórico
  const int16_t* hist;      // amostras válidas (mais antiga -> mais recente)
  size_t hist_count;        // amostras disponíveis em `hist`
};

// Escreve o payload em `out` (NUL-terminado quando cap > 0) e retorna o tamanho
// escrito sem o NUL. `idle_pct` é derivado de `load_pct` (clamp 0–100).
size_t buildStatusJson(char* out, size_t cap, const StatusSnapshot& s);

}  // namespace thermal
