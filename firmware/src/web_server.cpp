// web_server.cpp — rotas e semântica de resposta (design §4.5).
#include "web_server.h"

#include <Arduino.h>
#include <ESP8266WebServer.h>

#include "config.h"
#include "web/dashboard_html.h"

namespace web {
namespace {

ESP8266WebServer g_server(cfg::kHttpPort);
Handlers g_hooks = {nullptr, nullptr, nullptr};

void sendCommandResult(const CommandResult& r) {
  char buf[128];
  snprintf(buf, sizeof(buf),
           "{\"ok\":%s,\"reason\":\"%s\",\"pwm\":%u,\"latch\":%s,\"state\":\"%s\"}",
           r.ok ? "true" : "false", r.reason, static_cast<unsigned>(r.pwm),
           r.latch ? "true" : "false", r.state);
  g_server.send(200, F("application/json"), buf);
}

void handleRoot() { g_server.send_P(200, "text/html", DASHBOARD_HTML); }

void handleJson() {
  static char out[cfg::kJsonBufSize];
  g_hooks.fill_status(out, sizeof(out));
  g_server.send(200, F("application/json"), out);
}

void handleOn() { sendCommandResult(g_hooks.heater(true)); }
void handleOff() { sendCommandResult(g_hooks.heater(false)); }
void handleRearm() { sendCommandResult(g_hooks.rearm()); }

void handleNotFound() { g_server.send(404, F("text/plain"), F("404")); }

}  // namespace

void begin(const Handlers& hooks) {
  g_hooks = hooks;
  g_server.on("/", HTTP_GET, handleRoot);
  g_server.on("/json", HTTP_GET, handleJson);
  g_server.on("/on", HTTP_GET, handleOn);
  g_server.on("/off", HTTP_GET, handleOff);
  g_server.on("/rearm", HTTP_GET, handleRearm);
  g_server.onNotFound(handleNotFound);
  g_server.begin();
  Serial.printf("[web] HTTP na porta %u\n", static_cast<unsigned>(cfg::kHttpPort));
}

void poll() { g_server.handleClient(); }

}  // namespace web
