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

#include "hal/fsl_device_registers.h"
#include "wiring_digital.h"
#include "WString.h"

#include <cstddef>

typedef void (*DASH_IO_CALLBACK)(uint32_t io);

class DashClass
{
public:
    DashClass(uint32_t led, uint32_t en_5v, uint32_t v3_3, uint32_t wake, uint32_t wake_llwu);
    void begin();
    void end();

    void setLED(bool on);
    void toggleLED();
    void pulseLED(uint32_t on_ms, uint32_t off_ms);
    inline void onLED() {setLED(true);}
    inline void offLED() {setLED(false);}

    void enableIoInterrupt(uint32_t io, uint32_t config);
    void disableIoInterrupt(uint32_t io) {enableIoInterrupt(io, IO_INT_DISABLED);}
    void setIoCallback(DASH_IO_CALLBACK fx);
    void clearIoCallback() {setIoCallback(NULL);}

    void sleep();
    void deepSleep(uint32_t wakeUpPins) {lls(wakeUpPins, false);}
    void deepSleep() {deepSleep(0);}
    void shutdown(uint32_t wakeUpPins) {lls(wakeUpPins, true);}
    void shutdown() {shutdown(0);}

    //inline bool enabled5V() {return digitalRead(en_5v);}
    // bool enabled5V();
    // void set5V(bool enabled);

    //inline bool get3V3() {return !digitalRead(v3_3);}

    void pulseInterrupt();
    void ioInterrupt(uint32_t port);

    //TODO Boot Version? Read from flash
    int bootVersionNumber() {return *((uint32_t*)0x418);}
    String bootVersion();

protected:
    uint32_t led;
    uint32_t en_5v;
    uint32_t v3_3;
    uint32_t wake;
    uint32_t wake_llwu;
    uint32_t on_clocks;
    uint32_t off_clocks;
    bool pulse_on;
    bool ready;
    DASH_IO_CALLBACK ioCallback;

    void writeLED(bool on){digitalWrite(led, on ? HIGH : LOW);}
    void lls(uint32_t wakeUpPins, bool halt);
};
