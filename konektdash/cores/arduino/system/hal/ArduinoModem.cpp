/*
  ArduinoModem.cpp - Class definitions that provide Arduino layer for a modem
  interface on the Dash.

  https://hologram.io

  Copyright (c) 2017 Konekt, Inc.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
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

int ArduinoModem::modemavailable() {
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
