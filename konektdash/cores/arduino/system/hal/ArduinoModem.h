/*
  ArduinoModem.h - Class definitions that provide Arduino layer for a modem
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
#pragma once

#include "../sdk/network/modem/Modem.h"
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
    virtual int modemavailable();
    virtual uint8_t modemread();
    virtual uint8_t modempeek();
    virtual uint32_t msTick();

    Stream *uart;
    Stream *debug;
};
