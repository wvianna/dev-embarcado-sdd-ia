// dashboard_html.h — dashboard web embutida em PROGMEM (HTML/CSS/JS únicos).
//
// Requisitos: FR-021…FR-027 e FR-029 (design §4.5.1/ADR-010): pt-BR, tema claro,
// responsiva, sem rolagem em 1366×768, gauge + valor numérico, gráfico de
// tendência com escala FIXA de 20–90 °C (rótulos à esquerda, grade cinza,
// limite de 80 °C), botões ON/OFF com cor de estado, alertas de alarme/falha,
// latch + rearme, tooltips em todos os elementos e polling AJAX a cada 2 s (DEC-07).
// Sem CDN (o AP não tem internet) e sem frameworks.
#pragma once

static const char DASHBOARD_HTML[] PROGMEM = R"HTML(<!DOCTYPE html>
<html lang="pt-BR">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Monitoramento Térmico — ESP8266</title>
<style>
:root{--bg:#f4f7fa;--card:#fff;--line:#dfe6ee;--ink:#26313c;--muted:#6b7885;
--green:#1e9e57;--red:#d64545;--blue:#1f6fb2;--amber:#c7791a;}
*{box-sizing:border-box;margin:0;padding:0}
body{background:var(--bg);color:var(--ink);font:15px/1.4 system-ui,Segoe UI,Roboto,Arial,sans-serif}
.page{max-width:1340px;margin:0 auto;padding:10px 14px;display:grid;gap:10px}
.top{display:flex;align-items:center;justify-content:space-between;gap:12px;flex-wrap:wrap}
.top h1{font-size:19px}
.sub{color:var(--muted);font-size:12px}
.chips{display:flex;gap:8px;flex-wrap:wrap}
.chip{background:var(--card);border:1px solid var(--line);border-radius:999px;padding:4px 10px;
font-size:12px;color:var(--muted);white-space:nowrap}
.chip b{color:var(--ink);font-size:13px}
.main{display:grid;grid-template-columns:330px 1fr;gap:10px}
.card{background:var(--card);border:1px solid var(--line);border-radius:10px;padding:10px 12px}
.card h2{font-size:13px;color:var(--muted);font-weight:600;display:flex;justify-content:space-between;
align-items:center;margin-bottom:6px;text-transform:uppercase;letter-spacing:.4px}
.help{cursor:help;border:1px solid var(--line);border-radius:50%;width:16px;height:16px;display:inline-flex;
align-items:center;justify-content:center;font-size:11px;color:var(--muted)}
.temp{font-size:44px;font-weight:700;line-height:1.05}
.temp small{font-size:18px;font-weight:600;color:var(--muted)}
#gauge{display:block;width:100%;max-width:290px;margin:0 auto}
.badges{display:flex;gap:6px;flex-wrap:wrap;margin-top:6px}
.badge{border-radius:6px;padding:3px 8px;font-size:12px;border:1px solid var(--line);color:var(--muted)}
.badge.on{background:#e7f6ee;color:var(--green);border-color:#bfe6cf}
.badge.off{background:#fdecec;color:var(--red);border-color:#f3caca}
.badge.warn{background:#fdf3e3;color:var(--amber);border-color:#f0dcb6}
.badge.info{background:#e9f2fa;color:var(--blue);border-color:#c5ddf1}
#chart{display:block;width:100%;height:auto}
.bottom{display:grid;grid-template-columns:330px 1fr;gap:10px}
.buttons{display:flex;gap:8px;flex-wrap:wrap}
button{border:0;border-radius:8px;padding:9px 16px;font-size:14px;font-weight:600;cursor:pointer;color:#fff}
button.on{background:var(--green)}
button.off{background:var(--red)}
button.rearm{background:var(--amber)}
button:disabled{opacity:.45;cursor:not-allowed}
.feedback{margin-top:8px;font-size:12px;color:var(--muted);min-height:16px}
.feedback.err{color:var(--red)}
.alerts{display:grid;gap:6px}
.alert{border-radius:8px;padding:7px 10px;font-size:13px;display:none}
.alert.show{display:block}
.alert.alarm{background:#fdecec;color:#8f1f1f;border:1px solid #f0b9b9}
.alert.fault{background:#fdf3e3;color:#7a4a0e;border:1px solid #f0dcb6}
.alert.sensor{background:#eef1f4;color:#4a5560;border:1px solid #d7dde4}
.alert.latch{background:#fdecec;color:#8f1f1f;border:1px solid #f0b9b9;
display:none;align-items:center;justify-content:space-between;gap:8px}
.alert.latch.show{display:flex}
@media (max-width:900px){
.main,.bottom{grid-template-columns:1fr}
.chart-card{min-height:320px}
}
</style>
</head>
<body>
<div class="page">
  <header class="top">
    <div>
      <h1>Sistema de Monitoramento e Controle Térmico</h1>
      <div class="sub">NodeMCU v2 (ESP8266) · firmware <b id="fw">–</b> · AP <b id="ssid">–</b> · <b id="ip">–</b></div>
    </div>
    <div class="chips">
      <span class="chip" title="Tempo disponível do loop na última janela de 1 s (proxy de carga do firmware — ver design §5.1).">Idle <b id="idle">–</b></span>
      <span class="chip" title="Heap livre atual (ESP.getFreeHeap) e fragmentação.">RAM livre <b id="ram">–</b></span>
      <span class="chip" title="Ocupação do espaço de sketch (flash) e percentual do limite.">Flash <b id="flash">–</b></span>
      <span class="chip" title="Tempo desde o último boot.">Uptime <b id="uptime">–</b></span>
      <span class="chip" title="Clientes Wi-Fi associados ao AP.">Clientes <b id="clients">–</b></span>
    </div>
  </header>

  <section class="main">
    <div class="card">
      <h2>Temperatura <span class="help" title="Leitura do DS18B20 (GPIO4) a cada 1,2 s. '—' indica leitura inválida ou sensor ausente.">?</span></h2>
      <div class="temp"><span id="temp">—</span> <small id="temp-unit">°C</small></div>
      <canvas id="gauge" width="300" height="150" title="Termômetro de 20 °C a 90 °C. A faixa vermelha marca a fronteira de segurança de 80 °C."></canvas>
      <div class="badges">
        <span class="badge" id="b-state" title="Estado do sistema: Normal, Alarme latcheado, Falha de leitura ou Sensor ausente.">–</span>
        <span class="badge" id="b-sensor" title="Presença do sensor DS18B20 detectada pela varredura OneWire.">Sensor</span>
        <span class="badge" id="b-valid" title="Validade da última leitura (comunicação sem erro e dentro da faixa física).">Leitura</span>
      </div>
    </div>

    <div class="card chart-card">
      <h2>Tendência da temperatura <span class="help" title="Histórico das últimas leituras válidas (2 min). Escala fixa de 20 °C a 90 °C; linha vermelha = limite de 80 °C.">?</span></h2>
      <canvas id="chart" width="900" height="310" title="Gráfico de tendência com escala fixa de 20 a 90 °C, grade cinza a cada 10 °C e linha de limite em 80 °C."></canvas>
    </div>
  </section>

  <section class="bottom">
    <div class="card">
      <h2>Resistência (GPIO5 · PWM) <span class="help" title="Ligar aplica PWM 1023 (potência máxima); desligar aplica PWM 0. Comandos são recusados com motivo quando há bloqueio de segurança.">?</span></h2>
      <div class="buttons">
        <button class="on" id="btn-on" title="Ligar a resistência (PWM 1023).">Ligar</button>
        <button class="off" id="btn-off" title="Desligar a resistência (PWM 0).">Desligar</button>
      </div>
      <div class="feedback" id="feedback" title="Resultado do último comando enviado.">Pronto.</div>
      <div class="badges">
        <span class="badge" id="b-load" title="Estado aplicado à carga: ligada (verde) ou desligada (vermelho).">Carga</span>
        <span class="badge" id="b-pwm" title="PWM aplicado ao GPIO5 (0–1023).">PWM –</span>
      </div>
    </div>

    <div class="card alerts">
      <h2>Alertas <span class="help" title="Alarme térmico (≥ 80,0 °C), falha de sensor/leitura e estado do latch de segurança.">?</span></h2>
      <div class="alert alarm" id="alert-alarm" title="Temperatura ≥ 80,0 °C: resistência cortada imediatamente.">ALARME: temperatura ≥ 80,0 °C — resistência cortada.</div>
      <div class="alert fault" id="alert-fault" title="Falha de comunicação com o DS18B20: carga bloqueada.">Falha de leitura do sensor — carga bloqueada.</div>
      <div class="alert sensor" id="alert-sensor" title="Nenhum DS18B20 detectado no barramento: carga bloqueada.">Sensor ausente — carga bloqueada. Re-varredura a cada 5 s.</div>
      <div class="alert latch" id="alert-latch">
        <span>Alarme latcheado: rearme manual necessário (somente com leitura válida abaixo de 80 °C).</span>
        <button class="rearm" id="btn-rearm" title="Rearmar o alarme latcheado. Aceito apenas com leitura válida < 80,0 °C; a carga continua desligada até novo comando Ligar.">Rearmar</button>
      </div>
    </div>
  </section>
</div>

<script>
(function(){
"use strict";
var REASON={none:"",no_sensor:"sensor ausente",invalid_reading:"leitura inválida",
latched:"bloqueado pelo alarme latcheado",not_latched:"o latch não está ativo",
temp_high:"temperatura ≥ 80,0 °C"};
var $=function(id){return document.getElementById(id);};
var gauge=$("gauge"),gx=gauge.getContext("2d");
var chart=$("chart"),cx=chart.getContext("2d");
var T0=20,T1=90;

function setFeedback(msg,err){var f=$("feedback");f.textContent=msg;f.className="feedback"+(err?" err":"");}

function drawGauge(centi,valid){
  var w=gauge.width,h=gauge.height,cx0=w/2,cy0=h-16,r=Math.min(w/2-14,h-30);
  var a0=Math.PI,a1=2*Math.PI,f80=(80-T0)/(T1-T0);
  gx.clearRect(0,0,w,h);
  gx.lineWidth=14;gx.lineCap="round";
  gx.strokeStyle="#dbe2ea";gx.beginPath();gx.arc(cx0,cy0,r,a0,a1);gx.stroke();
  gx.strokeStyle="#e74c3c";gx.beginPath();gx.arc(cx0,cy0,r,a0+(a1-a0)*f80,a1);gx.stroke();
  gx.fillStyle="#7a8794";gx.font="12px system-ui";gx.textAlign="center";
  gx.fillText("20",cx0-r+6,cy0+18);gx.fillText("90",cx0+r-6,cy0+18);
  gx.fillStyle="#c0392b";gx.fillText("80",cx0+r*Math.cos(a0+(a1-a0)*f80),cy0+r*Math.sin(a0+(a1-a0)*f80)-8);
  var t=valid?Math.min(T1,Math.max(T0,centi/100)):T0;
  var ang=a0+(a1-a0)*((t-T0)/(T1-T0));
  gx.strokeStyle="#26313c";gx.lineWidth=3;
  gx.beginPath();gx.moveTo(cx0,cy0);gx.lineTo(cx0+Math.cos(ang)*r*0.85,cy0+Math.sin(ang)*r*0.85);gx.stroke();
  gx.fillStyle="#26313c";gx.beginPath();gx.arc(cx0,cy0,5,0,2*Math.PI);gx.fill();
}

function drawChart(hist){
  var w=chart.width,h=chart.height,padL=42,padR=12,padT=14,padB=12;
  var pw=w-padL-padR,ph=h-padT-padB;
  function y(t){return padT+ph*(1-(t-T0)/(T1-T0));}
  cx.clearRect(0,0,w,h);
  cx.font="12px system-ui";cx.textAlign="right";cx.textBaseline="middle";
  for(var t=T0;t<=T1;t+=10){
    var yy=y(t);
    cx.strokeStyle="#e6ebf1";cx.lineWidth=1;
    cx.beginPath();cx.moveTo(padL,yy);cx.lineTo(w-padR,yy);cx.stroke();
    cx.fillStyle="#7a8794";cx.fillText(t+"°",padL-6,yy);
  }
  cx.strokeStyle="#e74c3c";cx.setLineDash([6,5]);
  cx.beginPath();cx.moveTo(padL,y(80));cx.lineTo(w-padR,y(80));cx.stroke();
  cx.setLineDash([]);
  cx.fillStyle="#c0392b";cx.textAlign="left";cx.fillText("80 °C limite",padL+6,y(80)-9);
  if(!hist||hist.length===0){
    cx.fillStyle="#98a4b0";cx.textAlign="center";
    cx.fillText("sem leituras válidas",w/2,padT+ph/2);return;
  }
  var n=hist.length;
  function x(i){return n===1?padL+pw/2:padL+pw*i/(n-1);}
  cx.strokeStyle="#1f6fb2";cx.lineWidth=2;cx.lineJoin="round";
  cx.beginPath();
  for(var i=0;i<n;i++){
    var v=Math.min(T1,Math.max(T0,hist[i]/100)),xx=x(i),yy2=y(v);
    if(i===0){cx.moveTo(xx,yy2);}else{cx.lineTo(xx,yy2);}
  }
  cx.stroke();
  var last=Math.min(T1,Math.max(T0,hist[n-1]/100));
  cx.fillStyle="#1f6fb2";cx.beginPath();cx.arc(x(n-1),y(last),3.5,0,2*Math.PI);cx.fill();
}

function fmtUptime(ms){
  var s=Math.floor(ms/1000),h=Math.floor(s/3600),m=Math.floor((s%3600)/60),ss=s%60;
  return (h>0?h+"h ":"")+m+"m "+ss+"s";
}

var STATE_LABEL={NORMAL:["Normal","on"],LATCHED:["Alarme latcheado","off"],
INVALID_READING:["Falha de leitura","warn"],NO_SENSOR:["Sensor ausente","warn"]};

function apply(s){
  $("fw").textContent=s.fw;$("ssid").textContent=s.ssid;$("ip").textContent=s.ip;
  $("idle").textContent=s.idle_pct+"%";
  $("ram").textContent=(s.ram_free/1024).toFixed(0)+" KB ("+s.heap_frag+"% frag)";
  $("flash").textContent=(s.flash_used/1024).toFixed(0)+" KB ("+s.flash_pct+"%)";
  $("uptime").textContent=fmtUptime(s.uptime_ms);
  $("clients").textContent=s.clients;

  var tempTxt=s.valid?(s.temp/100).toFixed(2).replace(".",","):"—";
  $("temp").textContent=tempTxt;
  drawGauge(s.temp,s.valid);
  drawChart(s.hist);

  var st=STATE_LABEL[s.state]||[s.state,"info"];
  var bs=$("b-state");bs.textContent=st[0];bs.className="badge "+st[1];
  var sen=$("b-sensor");sen.textContent=s.sensor?"Sensor presente":"Sensor ausente";
  sen.className="badge "+(s.sensor?"on":"off");
  var val=$("b-valid");val.textContent=s.valid?"Leitura válida":"Leitura inválida";
  val.className="badge "+(s.valid?"on":"warn");

  var load=$("b-load"),on=(s.pwm>0);
  load.textContent=on?"Carga ligada":(s.block!=="none"?"Carga bloqueada":"Carga desligada");
  load.className="badge "+(on?"on":"off");
  var pw=$("b-pwm");pw.textContent="PWM "+s.pwm;
  pw.className="badge "+(on?"on":"off");

  $("alert-alarm").classList.toggle("show",!!s.alarm);
  $("alert-fault").classList.toggle("show",s.block==="invalid_reading");
  $("alert-sensor").classList.toggle("show",s.block==="no_sensor");
  var lt=$("alert-latch");lt.classList.toggle("show",!!s.latch);
  $("btn-rearm").disabled=!s.latch;
}

function refresh(){
  fetch("/json",{cache:"no-store"}).then(function(r){return r.json();}).then(apply)
  .catch(function(){setFeedback("Sem comunicação com o dispositivo.",true);});
}

function cmd(path){
  fetch(path,{cache:"no-store"}).then(function(r){return r.json();}).then(function(j){
    if(j.ok){setFeedback("Comando efetivado (PWM "+j.pwm+").",false);}
    else{setFeedback("Recusado: "+(REASON[j.reason]||j.reason)+".",true);}
    refresh();
  }).catch(function(){setFeedback("Falha ao enviar o comando.",true);});
}

// Botões: handlers ligados aqui dentro (o `cmd` do IIFE não é visível para
// `onclick` inline — ReferenceError em navegador real; correção 3ª sessão).
$("btn-on").addEventListener("click",function(){cmd("/on");});
$("btn-off").addEventListener("click",function(){cmd("/off");});
$("btn-rearm").addEventListener("click",function(){cmd("/rearm");});

setInterval(refresh,2000);  // polling de 2 s (FR-029/DEC-07)
refresh();
})();
</script>
</body>
</html>
)HTML";
