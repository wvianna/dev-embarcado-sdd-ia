#ifndef DASHBOARD_HTML_H
#define DASHBOARD_HTML_H

#include <Arduino.h>

// Dashboard servido em GET /. Mantido em PROGMEM para economizar RAM.
static const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Dashboard ESP8266</title>
<style>
  :root { color-scheme: dark; }
  body { font-family: system-ui, sans-serif; background: #0f172a; color: #e2e8f0;
         margin: 0; display: flex; min-height: 100vh; align-items: center; justify-content: center; }
  .card { background: #1e293b; border-radius: 16px; padding: 2rem; width: min(90vw, 420px);
          box-shadow: 0 10px 30px rgba(0,0,0,.4); }
  h1 { font-size: 1.2rem; margin: 0 0 1.2rem; color: #38bdf8; text-align: center; }
  .row { display: flex; justify-content: space-between; padding: .7rem 0; border-bottom: 1px solid #334155; }
  .row:last-child { border-bottom: none; }
  .label { color: #94a3b8; }
  .value { font-weight: 700; font-variant-numeric: tabular-nums; }
  .value.err { color: #f87171; }
  .meta { margin-top: 1rem; font-size: .78rem; color: #64748b; text-align: center; }
</style>
</head>
<body>
  <div class="card">
    <h1>Dashboard ESP8266</h1>
    <div class="row"><span class="label">Temperatura</span><span class="value" id="temp">—</span></div>
    <div class="row"><span class="label">ADC (A0)</span><span class="value" id="adc">—</span></div>
    <div class="row"><span class="label">Atualização</span><span class="value" id="ts">—</span></div>
    <div class="meta">Atualização automática a cada 1 s · 192.168.4.1</div>
  </div>
<script>
async function refresh() {
  try {
    const r = await fetch('/api/values');
    const d = await r.json();
    const tempEl = document.getElementById('temp');
    if (d.sensor_error) {
      tempEl.textContent = 'Erro de sensor';
      tempEl.className = 'value err';
    } else {
      tempEl.textContent = d.temperature.toFixed(1) + ' °C';
      tempEl.className = 'value';
    }
    document.getElementById('adc').textContent = d.adc;
    document.getElementById('ts').textContent = new Date(d.timestamp).toLocaleTimeString('pt-BR');
  } catch (e) {
    document.getElementById('temp').textContent = 'offline';
  }
}
refresh();
setInterval(refresh, 1000);
</script>
</body>
</html>
)rawliteral";

#endif // DASHBOARD_HTML_H
