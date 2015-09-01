/*
  Dash.cpp - Class definitions that provide special
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

#include "Dash.h"
#include "fsl_pit_hal.h"

#include "Arduino.h"

DashClass::DashClass(uint32_t led, uint32_t en_5v, uint32_t v3_3)
: led(led), en_5v(en_5v), v3_3(v3_3), on_clocks(0), off_clocks(0), pulse_on(0)
{}

void DashClass::begin()
{
    pinMode(led, OUTPUT);
    pinMode(en_5v, INPUT_PULLUP);
    pinMode(v3_3, INPUT);
    //setup PIT?
    SIM_HAL_EnableClock(SIM, kSimClockGatePit0);
    PIT_HAL_Enable(PIT);
    PIT_HAL_SetTimerRunInDebugCmd(PIT, false);
    NVIC_EnableIRQ(PIT0_IRQn);
}

void DashClass::end()
{
    NVIC_DisableIRQ(PIT0_IRQn);
    PIT_HAL_StopTimer(PIT, 0);
    //TODO Add disable port (mux0)?
    setLED(false);
    pinMode(en_5v, INPUT_PULLUP);
    pinMode(v3_3, INPUT);
}

void DashClass::setLED(bool on)
{
    PIT_HAL_StopTimer(PIT, 0);
    writeLED(on);
}

void DashClass::toggleLED()
{
    PIT_HAL_StopTimer(PIT, 0);
    digitalToggle(led);
}

void DashClass::pulseLED(uint32_t on_ms, uint32_t off_ms)
{
    PIT_HAL_StopTimer(PIT, 0);
    if(on_ms == 0)
    {
        writeLED(0);
    }
    else if(off_ms == 0)
    {
        writeLED(1);
    }
    else
    {
        on_clocks = SystemBusClock/1000 * on_ms;
        off_clocks = SystemBusClock/1000 * off_ms;

        PIT_HAL_SetTimerPeriodByCount(PIT, 0, on_clocks);
        pulse_on = true;
        writeLED(1);
        PIT_HAL_StartTimer(PIT, 0);
        PIT_HAL_SetIntCmd(PIT, 0, true);
    }
}

void DashClass::pulseInterrupt()
{
    PIT_HAL_StopTimer(PIT, 0);
    PIT_HAL_ClearIntFlag(PIT, 0);
    PIT_HAL_SetTimerPeriodByCount(PIT, 0, pulse_on ? off_clocks : on_clocks);
    pulse_on = ! pulse_on;
    writeLED(pulse_on);
    PIT_HAL_SetIntCmd(PIT, 0, true);
    PIT_HAL_StartTimer(PIT, 0);
}

// bool DashClass::enabled5V()
// {
//     return !((GPIO_RD_PDDR(GPIO_PORT(en_5v)) >> GPIO_PIN(en_5v)) & 1U);
// }
//
// void DashClass::set5V(bool enabled)
// {
//     pinMode(en_5v, OUTPUT);
//     digitalWrite(en_5v, enabled);
//     #if 0
//     if(enabled)
//         pinMode(en_5v, INPUT_PULLUP);
//     else
//     {
//         digitalWrite(en_5v, LOW);
//         pinMode(en_5v, OUTPUT);
//     }
//     #endif
// }
