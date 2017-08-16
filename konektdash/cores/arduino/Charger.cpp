/*
  Charger.cpp - Manual control of the battery charger.

  http://hologram.io

  Copyright (c) 2016 Konekt, Inc.  All rights reserved.

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

#include "Charger.h"
#include "Arduino.h"
#include "hal/fsl_pit_hal.h"

DashCharger::DashCharger(Max1704x &fuel_gauge)
: controllable(false), percentage_last(0), mv_last(0), interval(0),
  minute_count(0), recharge_pct(0), recharge_mv(0), gauge(&fuel_gauge)
{}

bool DashCharger::begin()
{
    if(ready) return controllable;
    ready = true;
    SIM_HAL_EnableClock(SIM, kSimClockGatePortB);
    PORT_WR_PCR_MUX(PORTB, 17, MUX_GPIO);
    PORT_WR_PCR_PS(PORTB, 17, PORT_PULL_UP);
    PORT_WR_PCR_PE(PORTB, 17, 1);
    GPIO_CLR_PDDR(PTB, 1u << 17);

    batteryPercentage();
    batteryMillivolts();

    controllable = (LOW == ((GPIO_RD_PDIR(PTB) >> 17) & 1U));

    if(controllable) {
        enable(false);
        Dash.snooze(3);
        enable(true);
    }

    interval = 0;
    recharge_pct = 0;
    recharge_mv = 0;

    return controllable;
}

void DashCharger::setupTimer(uint32_t minutes)
{
    interval = minutes;
    minute_count = 0;
    PIT_HAL_SetTimerPeriodByCount(PIT, 3, SystemBusClock * 60);
    NVIC_EnableIRQ(PIT3_IRQn);
    PIT_HAL_StartTimer(PIT, 3);
    PIT_HAL_SetIntCmd(PIT, 3, true);
}

void DashCharger::minuteInterrupt()
{
    PIT_HAL_ClearIntFlag(PIT, 3);
    if(minute_count < interval)
    {
        minute_count++;
    }
}

bool DashCharger::beginAutoPercentage(uint32_t minutes, uint8_t recharge_percentage)
{
    if(begin())
    {
        if(recharge_percentage > 99)
            recharge_percentage = 99;
        else if(recharge_percentage < 5)
            recharge_percentage = 5;
        recharge_pct = recharge_percentage;
        recharge_mv = 0;

        setupTimer(minutes);

        return true;
    }
    return false;
}

bool DashCharger::beginAutoMillivolts(uint32_t minutes, uint32_t recharge_millivolts)
{
    if(begin())
    {
        if(recharge_millivolts > 4100)
            recharge_millivolts = 4100;
        else if(recharge_millivolts < 3100)
            recharge_millivolts = 3100;
        recharge_mv = recharge_millivolts;
        recharge_pct = 0;

        setupTimer(minutes);

        return true;
    }
    return false;
}

void DashCharger::end()
{
    NVIC_DisableIRQ(PIT3_IRQn);
    PIT_HAL_StopTimer(PIT, 3);
    PIT_HAL_SetIntCmd(PIT, 3, false);
    ready = false;
    enable(true);
    PORT_WR_PCR_MUX(PORTB, 17, MUX_DISABLED);
    controllable = false;
    interval = 0;
    minute_count = 0;
}

bool DashCharger::isControllable()
{
    return controllable;
}

uint32_t DashCharger::batteryMillivolts()
{
    if(!ready) begin();
    mv_last = gauge->mv();
    return mv_last;
}

uint8_t DashCharger::batteryPercentage()
{
    if(!ready) begin();
    percentage_last = gauge->percentage();
    return percentage_last;
}

void DashCharger::checkAuto(bool force)
{
    if(!ready) begin();
    if(force || minute_count == interval)
    {
        minute_count = 0;

        if(recharge_pct)
            checkPercentage(recharge_pct);
        else if(recharge_mv)
            checkMillivolts(recharge_mv);
    }
}

bool DashCharger::isEnabled()
{
    if(!ready) begin();
    if(!controllable) return true;
    return ((GPIO_RD_PDDR(PTB) & (1<<17)) == 0);
}

void DashCharger::enable(bool enabled)
{
    if(!ready) begin();
    if(!controllable) return;
    if(enabled)
    {
        if(isEnabled())
        {
            //pulse low to restart a charger that is in fault mode
            GPIO_WR_PSOR(PTB, 1u << 17);
            GPIO_SET_PDDR(PTB, 1u << 17);
            delayMicroseconds(100);
        }
        PORT_WR_PCR_PE(PORTB, 17, 0);
        GPIO_CLR_PDDR(PTB, 1u << 17);
    }
    else
    {
        GPIO_WR_PSOR(PTB, 1u << 17);
        GPIO_SET_PDDR(PTB, 1u << 17);
    }
}

uint32_t DashCharger::checkPercentage(uint8_t restart_percentage)
{
    if(!ready) begin();
    batteryPercentage();
    batteryMillivolts();
    if(controllable)
    {
        if(percentage_last >= 100)
            enable(false);
        else if(percentage_last <= restart_percentage)
            enable(true);
    }
    return percentage_last;
}

uint32_t DashCharger::checkMillivolts(uint32_t restart_mv)
{
    if(!ready) begin();
    batteryPercentage();
    batteryMillivolts();
    if(controllable)
    {
        if(percentage_last >= 100)
            enable(false);
        else if(mv_last <= restart_mv)
            enable(true);
    }
    return mv_last;
}

uint8_t DashCharger::lastPercentage()
{
    return percentage_last;
}

uint32_t DashCharger::lastMillivolts()
{
    return mv_last;
}
