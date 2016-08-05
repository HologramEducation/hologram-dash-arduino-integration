/*
  Charger.h - Manual control of the battery charger.

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
#pragma once

#include "Arduino.h"

class DashCharger
{
public:
    DashCharger();
    bool begin();
    bool beginAutoPercentage(uint32_t minutes, uint8_t recharge_percentage=90);
    bool beginAutoMillivolts(uint32_t minutes, uint32_t recharge_millivolts=3900);
    void end();

    bool isEnabled();
    void enable(bool enabled);

    uint8_t lastPercentage();
    uint32_t lastMillivolts();

    uint32_t checkPercentage(uint8_t restart_percentage=90);
    uint32_t checkMillivolts(uint32_t restart_mv=3900);

    void checkAuto(bool force=false);

protected:
    bool charger_controllable;
    uint8_t percentage_last;
    uint32_t mv_last;
    uint32_t interval;
    uint32_t millis_count;
    uint32_t recharge_pct;
    uint32_t recharge_mv;
};

extern DashCharger Charger;
