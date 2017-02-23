#include "Clock.h"
#include <stdio.h>
#include "itoa.h"
#include "wiring_constants.h"

ClockClass::ClockClass()
: running(false), alarm_expired(true), was_reset(false) {}

bool ClockClass::isRunning()
{
    return running;
}

bool ClockClass::wasReset()
{
    return was_reset;
}

void ClockClass::getDateTime(rtc_datetime_t &dt)
{
    RTC_HAL_GetDatetime(RTC, &dt);
}

bool ClockClass::setDateTime(const rtc_datetime_t &dt)
{
    if(RTC_HAL_IsDatetimeCorrectFormat(&dt)) {
        RTC_HAL_SetDatetime(RTC, &dt);
        return true;
    }
    return false;
}

bool ClockClass::setDateTime(uint16_t year, uint16_t month, uint16_t day, uint16_t hour, uint16_t minute, uint16_t second)
{
    rtc_datetime_t dt;
    dt.year = year;
    dt.month = month;
    dt.day = day;
    dt.hour = hour;
    dt.minute = minute;
    dt.second = second;
    return setDateTime(dt);
}

bool ClockClass::setDate(uint16_t year, uint16_t month, uint16_t day)
{
    rtc_datetime_t dt;
    getDateTime(dt);
    dt.year = year;
    dt.month = month;
    dt.day = day;
    return setDateTime(dt);

}

bool ClockClass::setTime(uint16_t hour, uint16_t minute, uint16_t second)
{
    rtc_datetime_t dt;
    getDateTime(dt);
    dt.hour = hour;
    dt.minute = minute;
    dt.second = second;
    return setDateTime(dt);
}

String ClockClass::currentDateTime()
{
    rtc_datetime_t dt;
    getDateTime(dt);

    char buffer[20];
    sprintf(buffer, "%04u/%02u/%02u,%02u:%02u:%02u",
        dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
    return String(buffer);
}

String ClockClass::currentDate()
{
    rtc_datetime_t dt;
    getDateTime(dt);

    char buffer[12];
    sprintf(buffer, "%04u/%02u/%02u", dt.year, dt.month, dt.day);
    return String(buffer);
}

String ClockClass::currentTime()
{
    rtc_datetime_t dt;
    getDateTime(dt);

    char buffer[12];
    sprintf(buffer, "%02u:%02u:%02u", dt.hour, dt.minute, dt.second);
    return String(buffer);
}

//#define ENABLE_RTC_CLOCKOUT

void ClockClass::init()
{
    SIM_HAL_EnableClock(SIM, kSimClockGateRtc0);

#ifdef ENABLE_RTC_CLOCKOUT
    PORT_CLOCK_ENABLE(14);
    PORT_SET_MUX(14, MUX7);
    RTC_HAL_SetClockOutCmd(RTC, true);
#endif

    if(RTC_HAL_IsTimeInvalid(RTC))
    {
        RTC_HAL_SoftwareReset(RTC);
        RTC_HAL_SoftwareResetFlagClear(RTC);
        RTC_HAL_SetSecsReg(RTC, 1U);
    }

    NVIC_ClearPendingIRQ(RTC_IRQn);
    NVIC_ClearPendingIRQ(RTC_Seconds_IRQn);
    NVIC_EnableIRQ(RTC_IRQn);
    NVIC_EnableIRQ(RTC_Seconds_IRQn);

    if(!RTC_HAL_IsOscillatorEnabled(RTC) || !RTC_HAL_IsCounterEnabled(RTC)) {
        //sw reset?
        was_reset = true;
        RTC_HAL_SetCompensationIntervalRegister(RTC, 1);
        RTC_HAL_SetTimeCompensationRegister(RTC, 3); //Subtract 3 cycles from every 2 seconds
        RTC_HAL_SetOscillatorCmd(RTC, true);
        RTC_HAL_EnableCounter(RTC, true);
        adjust(-7);
    }
    RTC_HAL_SetSecsIntCmd(RTC, true);
    alarm_expired = RTC_HAL_HasAlarmOccured(RTC);
}

void ClockClass::adjust(int8_t ticks)
{
    RTC_HAL_SetTimeCompensationRegister(RTC, ticks);
}

int8_t ClockClass::adjusted()
{
    return (int8_t)RTC_HAL_GetTimeCompensationRegister(RTC);
}

uint32_t ClockClass::counter()
{
    return RTC_HAL_GetSecsReg(RTC);
}

void ClockClass::secondsInterrupt()
{
    RTC_HAL_SetSecsIntCmd(RTC, false);
    running = true;
}

void ClockClass::alarmInterrupt()
{
    if (RTC_HAL_HasAlarmOccured(RTC))
    {
        alarm_expired = true;
        // Disable follow-on interrupts
        RTC_HAL_SetAlarmIntCmd(RTC, false);
        if(alarm_callback)
            alarm_callback();
    }
}

void ClockClass::attachAlarmInterrupt(void (*callback)(void))
{
    alarm_callback = callback;
}

void ClockClass::cancelAlarm()
{
    RTC_HAL_SetAlarmReg(RTC, 0);
    RTC_HAL_SetAlarmIntCmd(RTC, false);
    alarm_expired = true;
}

bool ClockClass::setAlarm(uint32_t seconds)
{
    RTC_HAL_SetAlarmIntCmd(RTC, false);
    if(seconds > counter()) {
        alarm_expired = false;
        RTC_HAL_SetAlarmReg(RTC, seconds);
        RTC_HAL_SetAlarmIntCmd(RTC, true);
        return true;
    }
    return false;
}

bool ClockClass::setAlarm(const rtc_datetime_t &dt)
{
    if(RTC_HAL_IsDatetimeCorrectFormat(&dt)) {
        uint32_t seconds = 0;
        RTC_HAL_ConvertDatetimeToSecs(&dt, &seconds);
        return setAlarm(seconds);
    }
    return false;
}

bool ClockClass::setAlarmSecondsFromNow(uint32_t seconds)
{
    return setAlarm(counter()+seconds);
}

bool ClockClass::alarmExpired()
{
    return alarm_expired;
}
