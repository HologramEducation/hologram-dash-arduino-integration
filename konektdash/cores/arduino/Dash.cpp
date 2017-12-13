/*
  Dash.cpp - Class definitions that provide special
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

#include "Dash.h"
#include "hal/fsl_pit_hal.h"
#include "usb/SerialCDC.h"

#include "Arduino.h"
#include <climits>

DashClass::DashClass()
: on_clocks(0), off_clocks(0), pulse_on(0), ready(false),
  sleeping(false), wakeup(0), led_dim(0)
{}

void DashClass::begin()
{
    if(ready) return;
    pinMode(DASH_LED, OUTPUT);
    SIM_HAL_EnableClock(SIM, kSimClockGatePit0);
    PIT_HAL_Enable(PIT);
    PIT_HAL_SetTimerRunInDebugCmd(PIT, false);
    NVIC_EnableIRQ(PIT0_IRQn);
    NVIC_EnableIRQ(PIT2_IRQn);
    wakeup = 0;
    ready = true;
}

void DashClass::end()
{
    NVIC_DisableIRQ(PIT0_IRQn);
    NVIC_DisableIRQ(PIT1_IRQn);
    PIT_HAL_StopTimer(PIT, 0);
    PIT_HAL_StopTimer(PIT, 1);
    PIT_HAL_StopTimer(PIT, 2);
    wakeup = 0;
    setLED(false);
    ready = false;
}

void DashClass::stateLED(uint32_t *on, uint32_t *off, uint32_t *dim)
{
    if(on) *on = on_clocks;
    if(off) *off = off_clocks;
    if(dim) *dim = led_dim;
}

void DashClass::writeLED(bool on)
{
    if(led_dim)
        pinMode(DASH_LED, OUTPUT);
    led_dim = 0;
    digitalWrite(DASH_LED, on ? HIGH : LOW);
}

void DashClass::setLED(bool on)
{
    PIT_HAL_StopTimer(PIT, 0);
    on_clocks = 0;
    off_clocks = 0;
    writeLED(on);
}

void DashClass::toggleLED()
{
    PIT_HAL_StopTimer(PIT, 0);
    on_clocks = 0;
    off_clocks = 0;
    digitalToggle(DASH_LED);
}

void DashClass::dimLED(uint8_t percentage)
{
    if(percentage > 100) percentage = 100;

    if(percentage == 100 || percentage == 0) {
        setLED(percentage ? true : false);
    } else {
        PIT_HAL_StopTimer(PIT, 0);
        uint32_t v = ((uint32_t)(1<<getAnalogWriteResolution())) * percentage / 100;
        analogWrite(DASH_LED, v);
        led_dim = percentage;
    }
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

        PIT_HAL_SetTimerPeriodByCount(PIT, 0, on_clocks-1);
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
    PIT_HAL_SetTimerPeriodByCount(PIT, 0, (pulse_on ? off_clocks : on_clocks)-1);
    pulse_on = ! pulse_on;
    writeLED(pulse_on);
    PIT_HAL_SetIntCmd(PIT, 0, true);
    PIT_HAL_StartTimer(PIT, 0);
}

void DashClass::startSleepTimer(uint32_t ms)
{
    //use LPTMR, set prescaler, use 1kHz LPO (b01)
    uint32_t psr = (LPTMR_PSR_PCS(0x01) | LPTMR_PSR_PBYP_MASK);
    uint32_t cmr = ms;

    //default: no presaler
    if(ms > 65535)
    {
        //determine prescaler
        uint32_t prescale = 0;
        for(prescale=0; prescale<15; prescale++)
        {
            uint32_t overflow = 1 << (17+prescale);
            if(ms < overflow)
                break;
        }
        psr = (prescale << LPTMR_PSR_PRESCALE_SHIFT) | LPTMR_PSR_PCS(0x01);
        uint32_t ms_per_tick = 1 << (prescale+1);
        cmr = ms/ms_per_tick;
    }

    SIM_HAL_EnableClock(SIM, kSimClockGateLptmr0);
    LPTMR_WR_CSR(LPTMR0, 0x00); //Reset LPTMR
    LPTMR_WR_PSR(LPTMR0, psr); //Prescaler, LPO Clock Source
    LPTMR_WR_CMR(LPTMR0, cmr); //Compare value
    NVIC_EnableIRQ(LPTMR0_IRQn);
    LPTMR_WR_CSR(LPTMR0, LPTMR_CSR_TIE_MASK | LPTMR_CSR_TEN_MASK); //Clear Interrupt Flag, Enable Interrupt, Enable Timer
}

void DashClass::snooze(uint32_t ms)
{
    startSleepTimer(ms);

    SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk; //Wait mode on WFI
    //while(PIT_HAL_IsTimerRunning(PIT, 1))
    while(LPTMR_RD_CSR(LPTMR0))
    {
        __DSB();
        __WFI();
        __ISB();
        Charger.checkAuto();
    }
    SIM_HAL_DisableClock(SIM, kSimClockGateLptmr0);
}

void DashClass::sleep()
{
    SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk; //Wait mode on WFI
    sleeping = true;
    while(sleeping)
    {
        __DSB();
        __WFI();
        __ISB();
        Charger.checkAuto();
    }
}

void DashClass::wakeFromSleep()
{
    sleeping = false;
}

uint32_t DashClass::wakeupMask(uint32_t pin, uint32_t mode)
{
    if(IO_NOT_VALID(pin)) return 0;
    uint32_t w = 1 << (PINS_WAKEUP(pin) * 2);

    switch(mode)
    {
    case RISING: break;
    case FALLING:
        w = (w << 1);
        break;
    case CHANGE:
        w |= (w << 1);
        break;
    default: return 0;
    }
    return w;
}

void DashClass::attachWakeup(uint32_t pin, uint32_t mode)
{
    wakeup |= wakeupMask(pin, mode);
}

void DashClass::detachWakeup(uint32_t pin)
{
    wakeup &= ~wakeupMask(pin, CHANGE);
}

void DashClass::clearWakeup()
{
    wakeup = 0;
}

void DashClass::lls(bool halt, uint32_t mode)
{
    writeLED(0);

    Serial0.waitToEmpty();
    SerialSystem.waitToEmpty();
    Serial2.waitToEmpty();

    uint32_t usb = SerialUSB.ready();
    if(usb)
        SerialUSB.end();
    NVIC_EnableIRQ(LLWU_IRQn);
    if(mode == MODE_LLWU_LPTIMER)
    {
        LLWU_WR_PE1(LLWU_BASE_PTR, 0);   //Pins 0-3
        LLWU_WR_PE2(LLWU_BASE_PTR, 0);   //Pins 4-7
        LLWU_WR_PE3(LLWU_BASE_PTR, 0);   //Pins 8-11
        LLWU_WR_PE4(LLWU_BASE_PTR, 0);   //Pins 12-15
    }
    else
    {
        LLWU_WR_PE1(LLWU_BASE_PTR, wakeup & 0xFF);           //Pins 0-3
        LLWU_WR_PE2(LLWU_BASE_PTR, (wakeup >> 8) & 0xFF);    //Pins 4-7
        LLWU_WR_PE3(LLWU_BASE_PTR, (wakeup >> 16) & 0xFF);   //Pins 8-11
        LLWU_WR_PE4(LLWU_BASE_PTR, (wakeup >> 24) & 0xFF);   //Pins 12-15
    }

    if(mode == MODE_LLWU_GPIO)
    {
        LLWU_WR_ME(LLWU_BASE_PTR, 0x20);                     //Modules
        SMC_BWR_VLLSCTRL_VLLSM(SMC_BASE_PTR, 0); //VLLS0/LLS0 sub mode
    }
    else if(mode == MODE_LLWU_LPTIMER)
    {
        LLWU_WR_ME(LLWU_BASE_PTR, 1);    //LPTMR
        SMC_BWR_VLLSCTRL_VLLSM(SMC_BASE_PTR, 1); //VLLS1/LLS1 sub mode
    }
    else
    {
        LLWU_WR_ME(LLWU_BASE_PTR, 0x21);    //LPTMR
        SMC_BWR_VLLSCTRL_VLLSM(SMC_BASE_PTR, 1); //VLLS1/LLS1 sub mode
    }
    SMC_BWR_PMCTRL_STOPM(SMC_BASE_PTR, halt ? 4 : 3); //Enter VLLS or LLS mode on WFI
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;    //Stop mode on WFI
    __DSB();
    __WFI();
    __ISB();
    NVIC_DisableIRQ(LLWU_IRQn);
    delay(2);
    Charger.checkAuto(true);
    if(usb)
        SerialUSB.begin();
}

int DashClass::bootVersionNumber()
{
    uint8_t * pversion = (uint8_t*)0x418;
    return
    ((*pversion++) << 16) |
    ((*pversion++) <<  8) |
    ((*pversion++));
}

String DashClass::bootVersion()
{
    String major    = String(*(unsigned char*)0x418);
    String minor    = String(*(unsigned char*)0x419);
    String revision = String(*(unsigned char*)0x41A);
    return major + '.' + minor + '.' + revision;
}

String DashClass::serialNumber()
{
    char snbuf[33];

    sprintf(&snbuf[0], "%08X", SIM_UIDH);
    sprintf(&snbuf[8], "%08X", SIM_UIDMH);
    sprintf(&snbuf[16], "%08X", SIM_UIDML);
    sprintf(&snbuf[24], "%08X", SIM_UIDL);
    snbuf[32] = 0;
    return String(snbuf);
}

void DashClass::timedDeepSleep(uint32_t ms, bool all_sources_wake)
{
    startSleepTimer(ms);

    lls(false, all_sources_wake ? MODE_LLWU_ALL : MODE_LLWU_LPTIMER);

    LPTMR_WR_CSR(LPTMR0, 0x00); //Disable in case another event triggered wake
    SIM_HAL_DisableClock(SIM, kSimClockGateLptmr0);
    Charger.checkAuto(true);
}

void DashClass::deepSleepSec(uint32_t sec)
{
    if(sec > 4294967) return;
    timedDeepSleep(sec*1000, false);
}

void DashClass::deepSleepMin(uint32_t min)
{
    if(min > 71582) return;
    timedDeepSleep(min*60000, false);
}

void DashClass::deepSleepHour(uint32_t hour)
{
    if(hour > 1193) return;
    timedDeepSleep(hour*3600000, false);
}

void DashClass::deepSleepDay(uint32_t day)
{
    if(day > 49) return;
    timedDeepSleep(day*86400000, false);
}

void DashClass::deepSleepAtMostSec(uint32_t sec)
{
    if(sec > 4294967) return;
    timedDeepSleep(sec*1000, true);
}

void DashClass::deepSleepAtMostMin(uint32_t min)
{
    if(min > 71582) return;
    timedDeepSleep(min*60000, true);
}

void DashClass::deepSleepAtMostHour(uint32_t hour)
{
    if(hour > 1193) return;
    timedDeepSleep(hour*3600000, true);
}

void DashClass::deepSleepAtMostDay(uint32_t day)
{
    if(day > 49) return;
    timedDeepSleep(day*86400000, true);
}

void DashClass::attachTimer(void (*callback)(void))
{
    timer_callback = callback;
}

void DashClass::detachTimer()
{
    timer_callback = NULL;
}

void DashClass::startTimerMS(uint32_t interval_ms, bool repeat)
{
    //set T1 to 1 ms
    //set T2 to chain mode
    timer_oneshot = !repeat;
    PIT_HAL_SetTimerPeriodByCount(PIT, 2, interval_ms-1);
    PIT_HAL_SetIntCmd(PIT, 2, true);
    PIT_HAL_SetTimerChainCmd(PIT, 2, true);
    PIT_HAL_StartTimer(PIT, 2);

    PIT_HAL_SetTimerPeriodByCount(PIT, 1, SystemBusClock/1000-1);
    PIT_HAL_StartTimer(PIT, 1);
}

void DashClass::startTimerSec(uint32_t interval_sec, bool repeat)
{
    if(interval_sec < ULONG_MAX / 1000)
        startTimerMS(interval_sec*1000, repeat);
}

void DashClass::stopTimer()
{
    PIT_HAL_StopTimer(PIT, 1);
    PIT_HAL_ClearIntFlag(PIT, 1);
    PIT_HAL_StopTimer(PIT, 2);
    PIT_HAL_ClearIntFlag(PIT, 2);
}

bool DashClass::timerExpired() {
    return !PIT_HAL_IsTimerRunning(PIT, 2);
}

void DashClass::timerExpiration(uint32_t source)
{
    PIT_HAL_ClearIntFlag(PIT, source);
    if(timer_oneshot) {
        stopTimer();
    }
    if(timer_callback)
        timer_callback();
    wakeFromSleep();
}

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

static volatile uint32_t gLastWake = 0;

void LLWU_IRQHandler(void)
{
    gLastWake = (LLWU_F3 << 16) | (LLWU_F2 << 8) | LLWU_F1;

    LLWU_F1 = 0xFF;
    LLWU_F2 = 0xFF;

    if(LLWU_F3 & LLWU_F3_MWUF0_MASK)
    {
        if(LPTMR0_CSR & LPTMR_CSR_TCF_MASK)
        {
            LPTMR_SET_CSR(LPTMR0, LPTMR_CSR_TCF_MASK);
        }
    }

    if(LLWU_F3 & LLWU_F3_MWUF5_MASK) {
        RTC_HAL_SetAlarmIntCmd(RTC, false);
    }

    SystemInit();
}

#ifdef __cplusplus
}
#endif

uint8_t DashClass::lastWakeupSource()
{
    if(gLastWake == 0) return 0xFF;

    //count leading zeros
    //e.g. gLastWake is 0x00000010
    // CLZ is 27, returns 4
    return 32-__CLZ(gLastWake)-1;
}
