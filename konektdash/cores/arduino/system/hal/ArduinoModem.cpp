#include "ArduinoModem.h"
#include "delay.h"

ArduinoModem::ArduinoModem()
: uart(NULL) {
}

void ArduinoModem::begin(Stream &uart, URCReceiver &receiver, Stream *debug) {
    this->uart = &uart;
    this->debug = debug;
    init(receiver);
}

void ArduinoModem::modemout(const char* str) {
    debugout(str);
    uart->print(str);
}

void ArduinoModem::modemout(char c) {
    debugout(c);
    uart->write(c);
}

void ArduinoModem::modemout(uint8_t b) {
    if(debug) {
        switch(b) {
            case 0: debugout("\0"); break;
            case 0x10: debugout("\\n"); break;
            case 0x13: debugout("\\r"); break;
            default: debugout((char)b);
        }
    }
    uart->write(b);
}

void ArduinoModem::debugout(const char* str) {
    if(debug) {
        debug->print(str);
    }
}

void ArduinoModem::debugout(char c) {
    if(debug) {
        debug->print(c);
    }
}

void ArduinoModem::debugout(int i) {
    if(debug) {
        debug->print(i);
    }
}

bool ArduinoModem::modemavailable() {
    return uart->available();
}

uint8_t ArduinoModem::modemread() {
    return (uint8_t)uart->read();
}

uint8_t ArduinoModem::modempeek() {
    return (uint8_t)uart->peek();
}

uint32_t ArduinoModem::msTick() {
    return millis();
}

ArduinoModem modem;
