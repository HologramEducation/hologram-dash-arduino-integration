/*
  SerialCloud.h - Virutal serial port interface to HologramCloud

  http://hologram.io

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

#include "HardwareSerial.h"
#include "Hologram.h"
#include "RingBuffer.h"
#include "WString.h"

//WARNING! This class has been deprecated and will be removed in a future version.
//         Migrate to HologramCloud for additional functionality.
class SerialCloudClass : public HardwareSerial {
public:
    void begin(unsigned long);
    void begin(unsigned long baudrate, uint16_t config);
    void end();
    int available(void);
    int peek(void);
    int read(void);
    void flush(void);
    size_t write(uint8_t);
    operator bool() { return true; }
    void waitToEmpty(){}
    void pushSMS(const String &message);

protected:
    RingBuffer rxBuffer;

    void store(const char* str);
    void store(int i);
};
