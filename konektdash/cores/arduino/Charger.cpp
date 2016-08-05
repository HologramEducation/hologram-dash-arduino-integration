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

DashCharger::DashCharger()
: charger_controllable(false), percentage_last(0), mv_last(0), interval(0),
  millis_count(0), recharge_pct(0), recharge_mv(0)
{}

bool DashCharger::begin()
{
    SIM_HAL_EnableClock(SIM, kSimClockGatePortB);
    PORT_WR_PCR_MUX(PORTB, 17, MUX_GPIO);
    PORT_WR_PCR_PS(PORTB, 17, PORT_PULL_UP);
    PORT_WR_PCR_PE(PORTB, 17, 1);
    GPIO_CLR_PDDR(PTB, 1u << 17);

    delayMicroseconds(100);

    charger_controllable = (LOW == ((GPIO_RD_PDIR(PTB) >> 17) & 1U));

    percentage_last = Dash.batteryPercentage();
    mv_last = Dash.batteryMillivolts();

    interval = 0;
    recharge_pct = 0;
    recharge_mv = 0;

    return charger_controllable;
}

bool DashCharger::beginAutoPercentage(uint32_t minutes, uint8_t recharge_percentage)
{
    if(begin())
    {
        if(minutes > 71582)
            minutes = 71582;
        interval = minutes*60*1000;
        millis_count = millis();

        if(recharge_percentage > 99)
            recharge_percentage = 99;
        else if(recharge_percentage < 5)
            recharge_percentage = 5;
        recharge_pct = recharge_percentage;
        recharge_mv = 0;
        return true;
    }
    return false;
}

bool DashCharger::beginAutoMillivolts(uint32_t minutes, uint32_t recharge_millivolts)
{
    if(begin())
    {
        if(minutes > 71582)
            minutes = 71582;
        interval = minutes*60*1000;
        millis_count = millis();

        if(recharge_millivolts > 4100)
            recharge_millivolts = 4100;
        else if(recharge_millivolts < 3100)
            recharge_millivolts = 3100;
        recharge_mv = recharge_millivolts;
        recharge_pct = 0;
        return true;
    }
    return false;
}

void DashCharger::end()
{
    enable(true);
    PORT_WR_PCR_MUX(PORTB, 17, MUX_DISABLED);
    charger_controllable = false;
    interval = 0;
    millis_count = 0;
}

void DashCharger::checkAuto(bool force)
{
    if(force || (millis() - millis_count >= interval))
    {
        millis_count = millis();

        if(recharge_pct)
            checkPercentage(recharge_pct);
        else if(recharge_mv)
            checkMillivolts(recharge_mv);
    }
}

bool DashCharger::isEnabled()
{
    if(!charger_controllable) return true;
    return ((GPIO_RD_PDDR(PTB) & (1<<17)) == 0);
}

void DashCharger::enable(bool enabled)
{
    if(!charger_controllable) return;
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
    percentage_last = Dash.batteryPercentage();
    mv_last = Dash.batteryMillivolts();
    if(charger_controllable)
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
    percentage_last = Dash.batteryPercentage();
    mv_last = Dash.batteryMillivolts();
    if(charger_controllable)
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

DashCharger Charger;
