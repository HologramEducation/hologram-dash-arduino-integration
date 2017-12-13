/*
  Dash.h - Class definitions that provide special
  UI and features specific for the Konekt Dash and
  Konekt Dash Pro family of products.

  http://hologram.io

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
#include "Wire.h"

#include <cstddef>

class DashClass
{
public:
    DashClass();
    void begin();
    void end();

    void setLED(bool on);
    void toggleLED();
    void pulseLED(uint32_t on_ms, uint32_t off_ms);
    inline void onLED() {setLED(true);}
    inline void offLED() {setLED(false);}
    void dimLED(uint8_t percentage);

    void attachWakeup(uint32_t pin, uint32_t mode);
    void detachWakeup(uint32_t pin);
    void clearWakeup();

    void attachTimer(void (*callback)(void));
    void detachTimer();
    void startTimerMS(uint32_t interval_ms, bool repeat=false);
    void startTimerSec(uint32_t interval_sec, bool repeat=false);
    void stopTimer();
    bool timerExpired();

    void snooze(uint32_t ms);
    void sleep();
    void deepSleep() {lls(false);}
    void deepSleepSec(uint32_t sec);
    void deepSleepMin(uint32_t min);
    void deepSleepHour(uint32_t hour);
    void deepSleepDay(uint32_t day);
    void deepSleepAtMostSec(uint32_t sec);
    void deepSleepAtMostMin(uint32_t min);
    void deepSleepAtMostHour(uint32_t hour);
    void deepSleepAtMostDay(uint32_t day);
    void shutdown() {lls(true);}
    uint8_t lastWakeupSource();

    String serialNumber();

    void stateLED(uint32_t *on, uint32_t *off, uint32_t *dim);

    void pulseInterrupt();
    void wakeFromSleep();
    void timerExpiration(uint32_t source);

    int bootVersionNumber();
    String bootVersion();

protected:
    uint32_t on_clocks;
    uint32_t off_clocks;
    bool pulse_on;
    bool ready;
    volatile bool sleeping;
    uint32_t wakeup;
    uint8_t led_dim;
    void (*timer_callback)(void);
    bool timer_oneshot;

    enum
    {
        MODE_LLWU_GPIO,
        MODE_LLWU_LPTIMER,
        MODE_LLWU_ALL
    };

    void writeLED(bool on);
    void lls(bool halt, uint32_t mode=MODE_LLWU_GPIO);
    uint32_t wakeupMask(uint32_t pin, uint32_t mode);
    void startSleepTimer(uint32_t ms);
    void timedDeepSleep(uint32_t ms, bool all_sources_wake);
};
