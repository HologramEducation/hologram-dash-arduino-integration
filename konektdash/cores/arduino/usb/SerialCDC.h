/*
  SerialCDC.h - Implements SerialCDC class, which provides a USB serial port for
  Konekt Dash and Konekt Dash Pro family

  http://konekt.io

  Copyright (c) 2015 Konekt, Inc.  All rights reserved.


  Derived from file with original copyright notice:

  Copyright (c) 2015 Arduino LLC.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#pragma once

#include "RingBuffer.h"
#include "HardwareSerial.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "usb.h"
#include "usb_class_cdc.h"
#include "usb_descriptor.h"

#ifdef __cplusplus
}
#endif

#include <cstddef>

class SerialCDC : public HardwareSerial
{
public:
    SerialCDC();
    void fill(uint8_t data);
    void begin(unsigned long baudRate=115200);
    void begin(unsigned long baudrate, uint16_t config) {begin();}
    void flush();
    size_t write(const uint8_t data);
    virtual size_t write(const uint8_t *buffer, size_t size);
    void end();
    int available();
    int peek();
    int read();
    operator bool() { return true; }
    bool ready();
    using Print::write; // pull in write(str) and write(buf, size) from Print

    void waitToEmpty();

protected:
    RingBuffer rxBuffer;
    bool isReady;
};

extern SerialCDC SerialUSB;
