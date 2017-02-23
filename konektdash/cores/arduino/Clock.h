/*
  Clock.h - Class definitions that provide access to the
  Real Time Clock

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

#include "hal/fsl_rtc_hal.h"
#include "WString.h"

class ClockClass
{
public:
    ClockClass();
    void init();

    bool isRunning();
    bool wasReset();

    String currentDateTime();
    String currentDate();
    String currentTime();

    void getDateTime(rtc_datetime_t &dt);
    bool setDateTime(const rtc_datetime_t &dt);
    bool setDateTime(uint16_t year=1970, uint16_t month=1, uint16_t day=1, uint16_t hour=0, uint16_t minute=0, uint16_t second=0);
    bool setDate(uint16_t year=1970, uint16_t month=1, uint16_t day=1);
    bool setTime(uint16_t hour=0, uint16_t minute=0, uint16_t second=0);

    void attachAlarmInterrupt(void (*callback)(void));

    bool setAlarm(uint32_t seconds);
    bool setAlarm(const rtc_datetime_t &dt);
    bool setAlarmSecondsFromNow(uint32_t seconds);
    bool setAlarmMinutesFromNow(uint32_t minutes) {return setAlarmSecondsFromNow(minutes*60);}
    bool setAlarmHoursFromNow(uint32_t hours) {return setAlarmSecondsFromNow(hours*60*60);}
    bool setAlarmDaysFromNow(uint32_t days) {return setAlarmSecondsFromNow(days*24*60*60);}

    void cancelAlarm();
    bool alarmExpired();

    uint32_t counter();

    void adjust(int8_t ticks);
    int8_t adjusted();

    void alarmInterrupt();
    void secondsInterrupt();

protected:
    bool was_reset;
    volatile bool running;
    volatile uint32_t alarm_expired;
    void (*alarm_callback)(void);
};
