#include "web_server.h"
#include <json_formatter.h>
#include "web/dashboard_html.h"

WebServerService::WebServerService(const Sample* ref)
    : _server(80), _sample(ref) {}

void WebServerService::begin() {
    _server.on("/", HTTP_GET, [this]() { onIndex(); });
    _server.on("/api/values", HTTP_GET, [this]() { onApiValues(); });
    _server.begin();
}

void WebServerService::handleClient() {
    _server.handleClient();
}

void WebServerService::onIndex() {
    _server.send_P(200, "text/html", DASHBOARD_HTML);
}

void WebServerService::onApiValues() {
    const String payload = formatValuesJson(*_sample);
    _server.send(200, "application/json", payload);
}
