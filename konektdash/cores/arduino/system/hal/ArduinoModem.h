#pragma once

#include "../sdk/network/Modem.h"
#include "Stream.h"

class ArduinoModem : public Modem {
public:
    ArduinoModem();
    void begin(Stream &uart, URCReceiver &reciever, Stream *debug=NULL);

protected:
    virtual void modemout(char c);
    virtual void modemout(const char* str);
    virtual void modemout(uint8_t b);
    virtual void debugout(const char* str);
    virtual void debugout(char c);
    virtual void debugout(int i);
    virtual bool modemavailable();
    virtual uint8_t modemread();
    virtual uint8_t modempeek();
    virtual uint32_t msTick();

    Stream *uart;
    Stream *debug;
};

extern ArduinoModem modem;
