#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <sampler.h>

// Servidor HTTP: GET / (dashboard) e GET /api/values (JSON).
class WebServerService {
public:
    // ref: amostra mais recente, atualizada pelo loop de aquisição.
    explicit WebServerService(const Sample* ref);

    void begin();               // inicia o servidor na porta 80
    void handleClient();        // processa requisições (chamar no loop)

private:
    ESP8266WebServer _server;
    const Sample* _sample;
    void onIndex();
    void onApiValues();
};

#endif // WEB_SERVER_H
