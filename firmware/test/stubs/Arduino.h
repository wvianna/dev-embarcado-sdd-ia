#ifndef ARDUINO_STUB_H
#define ARDUINO_STUB_H

// Stub mínimo de Arduino para testes HOST (env native).
// Fornece apenas o necessário: tipos, String, A0, analogRead, millis, NAN.

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define A0 0
#define HIGH 0x1
#define LOW 0x0
#define INPUT 0x0
#define OUTPUT 0x1

// String mínima compatível com o código do firmware e ArduinoJson.
class String {
public:
    String() : _buf(nullptr), _len(0) {}
    String(const char* s) : _buf(nullptr), _len(0) { assign(s); }
    String(const String& o) : _buf(nullptr), _len(0) { assign(o.c_str()); }
    ~String() { free(_buf); }

    String& operator=(const char* s) { assign(s); return *this; }
    String& operator=(const String& o) { assign(o.c_str()); return *this; }

    const char* c_str() const { return _buf ? _buf : ""; }
    size_t length() const { return _len; }
    bool isEmpty() const { return _len == 0; }

    String& operator+=(const char* s) { append(s); return *this; }
    String& operator+=(char c) { char t[2] = {c, 0}; append(t); return *this; }
    String operator+(const char* s) const { String r(*this); r += s; return r; }

private:
    char* _buf;
    size_t _len;

    void assign(const char* s) {
        free(_buf);
        _buf = nullptr;
        _len = 0;
        if (!s) return;
        _len = strlen(s);
        _buf = static_cast<char*>(malloc(_len + 1));
        if (_buf) memcpy(_buf, s, _len + 1);
    }
    void append(const char* s) {
        if (!s) return;
        size_t n = strlen(s);
        char* nb = static_cast<char*>(realloc(_buf, _len + n + 1));
        if (!nb) return;
        memcpy(nb + _len, s, n + 1);
        _buf = nb;
        _len += n;
    }
};

static inline uint32_t millis() { return 0; }
static inline int analogRead(uint8_t) { return 0; }

#ifndef NAN
#define NAN (0.0f / 0.0f)
#endif

#endif // ARDUINO_STUB_H
