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
  :root { color-scheme: dark; --bg: #0f172a; --panel: #1e293b; --line: #334155;
          --text: #e2e8f0; --muted: #94a3b8; --cyan: #38bdf8; --green: #34d399; }
  * { box-sizing: border-box; }
    body { font-family: system-ui, sans-serif; background: var(--bg); color: var(--text);
      margin: 0; min-height: 100vh; padding: .65rem; }
    main { margin: auto; width: min(100%, 900px); }
    h1 { font-size: 1.15rem; margin: 0 0 .55rem; color: var(--cyan); }
    .grid { display: grid; grid-template-columns: 1fr 1fr; gap: .65rem; }
  .panel { background: var(--panel); border: 1px solid var(--line); border-radius: 12px;
        padding: .65rem; box-shadow: 0 8px 24px rgba(0,0,0,.25); min-width: 0; }
  .label { color: var(--muted); font-size: .82rem; }
  .value { display: block; color: var(--green); font-size: 1.8rem; font-weight: 700;
           font-variant-numeric: tabular-nums; margin-top: .25rem; }
  .value.err { color: #f87171; font-size: 1.1rem; }
  canvas { display: block; width: 100%; height: auto; }
  .meta { margin-top: .55rem; font-size: .72rem; color: #64748b; text-align: center; }
  @media (max-width: 560px) { .grid { grid-template-columns: 1fr; } }
</style>
</head>
<body>
  <main>
    <h1>Dashboard ESP8266</h1>
    <div class="grid">
      <section class="panel"><span class="label">Temperatura</span><span class="value" id="temp">—</span></section>
      <section class="panel"><span class="label">ADC (A0)</span><span class="value" id="adc">—</span></section>
      <section class="panel"><span class="label">Gauge de temperatura (20–40 °C)</span><canvas id="tempGauge" width="700" height="180"></canvas></section>
      <section class="panel"><span class="label">Gauge do sinal analógico (0–1023)</span><canvas id="adcGauge" width="700" height="180"></canvas></section>
      <section class="panel"><span class="label">Tendência de temperatura (20–40 °C)</span><canvas id="tempTrend" width="700" height="240"></canvas></section>
      <section class="panel"><span class="label">Tendência do sinal analógico (0–1023)</span><canvas id="adcTrend" width="700" height="240"></canvas></section>
    </div>
    <div class="meta">Atualização automática a cada 1 s · 192.168.4.1</div>
  </main>
<script>
const tempHistory = [];
const adcHistory = [];
const maxHistory = 60;

function drawGauge(id, value, min, max, unit, error) {
  const canvas = document.getElementById(id);
  const ctx = canvas.getContext('2d');
  const width = canvas.width;
  const height = canvas.height;
  const cx = width / 2;
  const cy = height - 18;
  const radius = Math.min(width * .34, height * .78);
  const start = Math.PI;
  const end = 2 * Math.PI;
  ctx.clearRect(0, 0, width, height);
  ctx.lineWidth = 20;
  ctx.lineCap = 'round';
  ctx.strokeStyle = '#334155';
  ctx.beginPath(); ctx.arc(cx, cy, radius, start, end); ctx.stroke();
  if (!error) {
    const ratio = Math.max(0, Math.min(1, (value - min) / (max - min)));
    ctx.strokeStyle = '#34d399';
    ctx.beginPath(); ctx.arc(cx, cy, radius, start, start + ratio * Math.PI); ctx.stroke();
    ctx.fillStyle = '#e2e8f0';
    ctx.font = 'bold 24px system-ui';
    ctx.textAlign = 'center';
    ctx.fillText(value.toFixed(unit === '°C' ? 1 : 0) + ' ' + unit, cx, cy - 22);
  } else {
    ctx.fillStyle = '#f87171';
    ctx.font = 'bold 18px system-ui';
    ctx.textAlign = 'center';
    ctx.fillText('Erro de sensor', cx, cy - 22);
  }
  ctx.fillStyle = '#94a3b8';
  ctx.font = '12px system-ui';
  ctx.textAlign = 'left'; ctx.fillText(min + ' ' + unit, cx - radius, height - 2);
  ctx.textAlign = 'right'; ctx.fillText(max + ' ' + unit, cx + radius, height - 2);
}

function drawTrend(id, points, min, max, unit, color) {
  const canvas = document.getElementById(id);
  const ctx = canvas.getContext('2d');
  const width = canvas.width;
  const height = canvas.height;
  const pad = 28;
  ctx.clearRect(0, 0, width, height);
  ctx.strokeStyle = '#334155';
  ctx.lineWidth = 1;
  ctx.beginPath(); ctx.moveTo(pad, pad); ctx.lineTo(pad, height - pad); ctx.lineTo(width - pad, height - pad); ctx.stroke();
  if (points.length === 0) return;
  ctx.strokeStyle = color;
  ctx.lineWidth = 3;
  ctx.beginPath();
  points.forEach((point, index) => {
    const x = pad + (width - 2 * pad) * (index / Math.max(1, maxHistory - 1));
    const y = height - pad - (height - 2 * pad) * ((point.value - min) / Math.max(.1, max - min));
    if (index === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y);
  });
  ctx.stroke();
  ctx.fillStyle = '#94a3b8';
  ctx.font = '12px system-ui';
  ctx.textAlign = 'left'; ctx.fillText(max + ' ' + unit, pad, pad - 8);
  ctx.textAlign = 'left'; ctx.fillText(min + ' ' + unit, pad, height - 4);
}

async function refresh() {
  try {
    const r = await fetch('/api/values');
    const d = await r.json();
    const tempEl = document.getElementById('temp');
    if (d.sensor_error) {
      tempEl.textContent = 'Erro de sensor';
      tempEl.className = 'value err';
      drawGauge('tempGauge', 0, 20, 40, '°C', true);
      drawTrend('tempTrend', tempHistory, 20, 40, '°C', '#38bdf8');
    } else {
      tempEl.textContent = d.temperature.toFixed(1) + ' °C';
      tempEl.className = 'value';
      tempHistory.push({ value: d.temperature, time: d.timestamp });
      if (tempHistory.length > maxHistory) tempHistory.shift();
      drawGauge('tempGauge', d.temperature, 20, 40, '°C', false);
      drawTrend('tempTrend', tempHistory, 20, 40, '°C', '#38bdf8');
    }
    document.getElementById('adc').textContent = d.adc;
    adcHistory.push({ value: d.adc, time: d.timestamp });
    if (adcHistory.length > maxHistory) adcHistory.shift();
    drawGauge('adcGauge', d.adc, 0, 1023, '', false);
    drawTrend('adcTrend', adcHistory, 0, 1023, '', '#fbbf24');
  } catch (e) {
    document.getElementById('temp').textContent = 'offline';
    drawGauge('tempGauge', 0, 20, 40, '°C', true);
  }
}
drawGauge('tempGauge', 0, 20, 40, '°C', true);
drawGauge('adcGauge', 0, 0, 1023, '', false);
drawTrend('tempTrend', tempHistory, 20, 40, '°C', '#38bdf8');
drawTrend('adcTrend', adcHistory, 0, 1023, '', '#fbbf24');
refresh();
setInterval(refresh, 1000);
</script>
</body>
</html>
)rawliteral";

#endif // DASHBOARD_HTML_H
