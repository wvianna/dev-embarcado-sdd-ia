#ifndef DS18B20_DRIVER_H
#define DS18B20_DRIVER_H

#include <Arduino.h>
#include <OneWire.h>

// Driver para o sensor DS18B20 via OneWire no GPIO4 (D2).
class Ds18b20Driver {
public:
    // gpio: pino OneWire (default D2/GPIO4 no NodeMCU V2)
    explicit Ds18b20Driver(uint8_t gpio = 4);

    // Inicializa o barramento e o sensor. Retorna true se um dispositivo foi encontrado.
    bool begin();

    // Lê a temperatura em °C. Em falha, retorna NAN e marca erro.
    float readCelsius(bool* ok = nullptr);

    // Último estado de erro.
    bool lastError() const { return _error; }

    // Número de dispositivos encontrados no barramento no último scan.
    uint8_t deviceCount() const { return _deviceCount; }

    // Endereço ROM (64 bits) do dispositivo alvo, se encontrado.
    const uint8_t* address() const { return _found ? _address : nullptr; }

private:
    uint8_t _gpio;
    bool _error;
    bool _ready;
    bool _found;
    uint8_t _deviceCount;
    uint8_t _address[8];      // endereço ROM (64 bits) do sensor alvo

    // Varre o barramento e registra os endereços encontrados no log serial.
    void scanAndLog();
};

#endif // DS18B20_DRIVER_H
