/*
  Dash.h - Class definitions that provide special
  UI and features specific for the Konekt Dash and
  Konekt Dash Pro family of products.
  
  http://konekt.io
  
  Copyright (c) 2015 Konekt, Inc.  All rights reserved.

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

#include "fsl_device_registers.h"
#include "wiring_digital.h"

#include <cstddef>

class DashClass
{
public:
    DashClass(uint32_t led, uint32_t en_5v, uint32_t v3_3);
    void begin();
    void end();

    void setLED(bool on);
    void toggleLED();
    void pulseLED(uint32_t on_ms, uint32_t off_ms);
    inline void onLED() {setLED(true);}
    inline void offLED() {setLED(false);}
    void pulseInterrupt();

    //inline bool enabled5V() {return digitalRead(en_5v);}
    // bool enabled5V();
    // void set5V(bool enabled);

    //inline bool get3V3() {return !digitalRead(v3_3);}

    //TODO Boot Version? Read from flash
protected:
    uint32_t led;
    uint32_t en_5v;
    uint32_t v3_3;
    uint32_t on_clocks;
    uint32_t off_clocks;
    bool pulse_on;

    void writeLED(bool on){digitalWrite(led, on ? HIGH : LOW);}
};
