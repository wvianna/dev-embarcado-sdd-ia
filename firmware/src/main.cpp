// main.cpp — composição do firmware: amostragem, segurança, atuação, rede e métricas.
//
// Ordem do loop e contratos: .specs/features/controle-termico/design.md §4.2/§4.3.
// Contexto único (loop principal), sem ISR/task (design §5); buffers estáticos (NFR-002).
#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "config.h"
#include "console.h"
#include "ds18b20_sensor.h"
#include "web_server.h"

#include "buzzer_pattern.h"
#include "control_policy.h"
#include "periodic_timer.h"
#include "status_json.h"
#include "trend_buffer.h"

namespace {

// --- Estado com owner único (loop principal) ---
thermal::ControlPolicy g_policy;
thermal::BuzzerPattern g_buzzer;
thermal::PeriodicTimer g_sample_timer;
thermal::PeriodicTimer g_rescan_timer;

int16_t g_trend_storage[cfg::kTrendCapacity];
thermal::TrendBuffer g_trend(g_trend_storage, cfg::kTrendCapacity);
int16_t g_hist_scratch[cfg::kTrendCapacity];  // cópia linear para o payload /json

char g_json_buf[cfg::kJsonBufSize];
char g_ssid[24] = "ESP8266_------";
char g_reset_reason[32] = "unknown";

// Última leitura válida: a temperatura apresentada fica congelada quando a leitura
// falha (nunca inventa valor — FR-014/CA-017).
bool g_has_valid = false;
int16_t g_last_valid_centi = 0;

// Estatísticas de amostragem (evidência CA-004).
uint32_t g_last_sample_ms = 0;
uint32_t g_samp_ms = 0;
uint32_t g_samp_min_ms = 0;
uint32_t g_samp_max_ms = 0;

// Janela dos últimos 60 intervalos (jitter em regime, pós-transiente de boot).
constexpr size_t kSampWindow = 60;
uint16_t g_samp_ring[kSampWindow];
size_t g_samp_ring_head = 0;
size_t g_samp_ring_count = 0;

// Métrica de carga/idle por janela de 1 s (proxy documentado — design §5.1).
uint32_t g_busy_us = 0;
uint32_t g_window_start_ms = 0;
uint8_t g_load_pct = 0;

// --- Comandos: fonte única é a política (HTTP e console passam por aqui) ---
web::CommandResult applyHeater(bool on) {
  const thermal::CommandOutcome r = g_policy.requestHeater(on);
  return {r.accepted, thermal::reasonName(r.reason), r.pwm, r.latch, g_policy.stateName()};
}

web::CommandResult applyRearm() {
  const thermal::CommandOutcome r = g_policy.requestRearm();
  return {r.accepted, thermal::reasonName(r.reason), r.pwm, r.latch, g_policy.stateName()};
}

// --- Telemetria: contrato /json (design §3.5) ---
size_t fillStatusJson(char* out, size_t cap) {
  const size_t count = g_trend.size();
  for (size_t i = 0; i < count; ++i) {
    g_hist_scratch[i] = g_trend.at(i);
  }

  // Janela de jitter: mínimo/máximo dos últimos ≤60 intervalos registrados.
  uint32_t win_min = 0;
  uint32_t win_max = 0;
  for (size_t i = 0; i < g_samp_ring_count; ++i) {
    const uint32_t v = g_samp_ring[i];
    if (i == 0 || v < win_min) {
      win_min = v;
    }
    if (v > win_max) {
      win_max = v;
    }
  }

  thermal::StatusSnapshot s = {};
  s.fw = cfg::kFwVersion;
  s.uptime_ms = millis();
  s.state = g_policy.stateName();
  s.sensor = sensor::present();
  s.valid = g_policy.inputs().reading_valid;
  s.temp_centi = g_has_valid ? g_last_valid_centi : 0;
  s.pwm = g_policy.pwm();
  s.latch = g_policy.latch();
  s.alarm = g_policy.alarm();
  s.block = thermal::reasonName(g_policy.block_reason());
  s.load_pct = g_load_pct;
  s.ram_free = ESP.getFreeHeap();
  s.heap_frag = static_cast<uint8_t>(ESP.getHeapFragmentation());
  s.flash_used = ESP.getSketchSize();
  s.flash_pct =
      static_cast<uint8_t>(static_cast<uint64_t>(s.flash_used) * 100 / cfg::kSketchSpaceBytes);
  s.samp_ms = g_samp_ms;
  s.samp_min_ms = g_samp_min_ms;
  s.samp_max_ms = g_samp_max_ms;
  s.samp_win_min_ms = win_min;
  s.samp_win_max_ms = win_max;
  s.ssid = g_ssid;
  s.ip = "192.168.4.1";
  s.clients = static_cast<uint8_t>(WiFi.softAPgetStationNum());
  s.reset_reason = g_reset_reason;
  s.hist_period_ms = cfg::kSamplePeriodMs;
  s.hist = g_hist_scratch;
  s.hist_count = count;
  return thermal::buildStatusJson(out, cap, s);
}

// --- Amostragem de 1,2 s (FR-004/FR-005/FR-006; DEC-07) ---
void handleSample(uint32_t now) {
  sensor::Reading r = sensor::read();  // resultado da conversão anterior (sem espera)

#if defined(BENCH_TEMP_INJECTION)
  if (console::injectionActive()) {  // bancada: injeção substitui a leitura física
    r.valid = console::injectionValid();
    r.centi = console::injectionCenti();
  }
#endif

  if (g_last_sample_ms != 0) {  // intervalo entre amostras (evidência CA-004)
    g_samp_ms = now - g_last_sample_ms;
    if (g_samp_min_ms == 0 || g_samp_ms < g_samp_min_ms) {
      g_samp_min_ms = g_samp_ms;
    }
    if (g_samp_ms > g_samp_max_ms) {
      g_samp_max_ms = g_samp_ms;
    }
    g_samp_ring[g_samp_ring_head] =
        static_cast<uint16_t>(g_samp_ms > 65535u ? 65535u : g_samp_ms);
    g_samp_ring_head = (g_samp_ring_head + 1) % kSampWindow;
    if (g_samp_ring_count < kSampWindow) {
      ++g_samp_ring_count;
    }
  }
  g_last_sample_ms = now;

  bool present = sensor::present();
#if defined(BENCH_TEMP_INJECTION)
  if (console::injectionActive()) {
    present = true;  // a injeção simula sensor presente
  }
#endif

  g_policy.update(thermal::PolicyInputs{present, r.valid, r.centi});
  if (r.valid) {
    g_last_valid_centi = r.centi;
    g_has_valid = true;
    g_trend.push(r.centi);  // somente leituras válidas entram no histórico
  }
  sensor::startConversion();  // re-dispara imediatamente a próxima conversão (FR-005)
}

}  // namespace

void setup() {
  Serial.begin(cfg::kSerialBaud);

  // Motivo do reset registrado quando disponível (FR-017); uso único de String no setup.
  snprintf(g_reset_reason, sizeof(g_reset_reason), "%s", ESP.getResetReason().c_str());
  Serial.printf("\n[boot] monitor termico v%s — reset: %s\n", cfg::kFwVersion, g_reset_reason);

  // Pinos em estado seguro antes de qualquer outra inicialização.
  pinMode(cfg::kPinHeater, OUTPUT);
  analogWriteRange(cfg::kHeaterPwmMax);
  analogWrite(cfg::kPinHeater, 0);
  pinMode(cfg::kPinBuzzer, OUTPUT);
  digitalWrite(cfg::kPinBuzzer, LOW);

  g_policy.begin();  // carga desligada, latch limpo, bloqueio até leitura válida (DEC-02)
  g_buzzer.begin(millis());
  g_sample_timer.begin(millis(), cfg::kSamplePeriodMs);
  g_rescan_timer.begin(millis(), cfg::kSensorRescanMs);
  g_window_start_ms = millis();

  sensor::begin();  // varredura OneWire do boot + vínculo da ROM (FR-001)
  if (sensor::present()) {
    sensor::startConversion();  // primeira conversão já disparada no boot
  }

  // AP aberto com SSID derivado do MAC (FR-018/FR-019, ADR-011).
  WiFi.persistent(false);  // sem gravação de configuração na flash
  WiFi.mode(WIFI_AP);
  // Modem sleep desativado: o ciclo de power-save do rádio adicionava ~3 s à
  // primeira / após ociosidade, violando NFR-004 (latência ≤ 500 ms).
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  uint8_t mac[6];
  WiFi.macAddress(mac);
  snprintf(g_ssid, sizeof(g_ssid), "ESP8266_%02X%02X%02X", mac[3], mac[4], mac[5]);
  IPAddress ip(cfg::kApIp[0], cfg::kApIp[1], cfg::kApIp[2], cfg::kApIp[3]);
  IPAddress mask(cfg::kApMask[0], cfg::kApMask[1], cfg::kApMask[2], cfg::kApMask[3]);
  WiFi.softAPConfig(ip, ip, mask);
  WiFi.softAP(g_ssid);  // rede aberta: decisão de produto registrada
  Serial.printf("[wifi] AP %s em %s (DHCP ativo)\n", g_ssid, WiFi.softAPIP().toString().c_str());

  web::begin({applyHeater, applyRearm, fillStatusJson});
#if defined(BENCH_TEMP_INJECTION)
  console::begin({applyHeater, applyRearm, fillStatusJson, sensor::rescan});
#endif

  Serial.println("[boot] pronto");
}

void loop() {
  const uint32_t now = millis();

  // Fecha a janela de carga/idle a cada 1 s (design §5.1).
  if (static_cast<uint32_t>(now - g_window_start_ms) >= cfg::kHealthWindowMs) {
    const uint32_t total_us = static_cast<uint32_t>(now - g_window_start_ms) * 1000u;
    if (total_us > 0) {
      const uint32_t pct =
          static_cast<uint32_t>(static_cast<uint64_t>(g_busy_us) * 100ull / total_us);
      g_load_pct = static_cast<uint8_t>(pct > 100 ? 100 : pct);
    }
    g_busy_us = 0;
    g_window_start_ms = now;
  }

  // 1) Amostragem: leitura do ciclo anterior + re-disparo imediato da conversão.
  if (g_sample_timer.consume(now)) {
    const uint32_t t0 = micros();
    handleSample(now);
    g_busy_us += micros() - t0;
  }

  // 2) Re-varredura OneWire (5 s) — sensor ausente sai do estado automaticamente.
  if (g_rescan_timer.consume(now)) {
    const uint32_t t0 = micros();
    sensor::rescan();
    g_busy_us += micros() - t0;
  }

  // 3) HTTP cooperativo (sem bloqueio; dashboard e /json respondem sempre).
  {
    const uint32_t t0 = micros();
    web::poll();
    g_busy_us += micros() - t0;
  }

  // 4) Console de bancada (compilado apenas no env `bancada`).
#if defined(BENCH_TEMP_INJECTION)
  {
    const uint32_t t0 = micros();
    console::poll();
    g_busy_us += micros() - t0;
  }
#endif

  // 5) Atuação na mesma avaliação da amostra (corte imediato — FR-010).
  analogWrite(cfg::kPinHeater, g_policy.pwm());
  digitalWrite(cfg::kPinBuzzer, g_buzzer.update(now, g_policy.alarm()) ? HIGH : LOW);
}
