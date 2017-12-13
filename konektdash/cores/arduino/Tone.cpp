/*
  Tone.cpp - Implements tone and related functions,
  with mods for the Konekt Dash and Konekt Dash Pro family

  https://hologram.io

  Copyright (c) 2017 Konekt, Inc.  All rights reserved.


  Derived from file with original copyright notice:

  Copyright (c) 2015 Arduino LLC.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "Tone.h"
#include "Arduino.h"
#include "wiring_constants.h"
#include "hal/fsl_ftm_hal.h"

#define PWM_FTM(inst)       ((inst) == PWM_3 ? FTM3 : ((inst) == PWM_2 ? FTM2 : ((inst) == PWM_1 ? FTM1 : FTM0)))
#define PWM_IRQ(inst)       ((inst) == PWM_3 ? FTM3_IRQn : ((inst) == PWM_2 ? FTM2_IRQn : ((inst) == PWM_1 ? FTM1_IRQn : FTM0_IRQn)))
#define PWM_GATE(inst)      ((inst) == PWM_3 ? kSimClockGateFtm3 : ((inst) == PWM_2 ? kSimClockGateFtm2 : ((inst) == PWM_1 ? kSimClockGateFtm1 : kSimClockGateFtm0)))
#define PWM_CHANNELS(inst)  (((inst) == PWM_1 || (inst) == PWM_2) ? 2 : 8)

static volatile uint32_t cycles = 0;
static void (*tone_callback)(void);

bool tonePlaying() {
    return cycles > 0;
}

void noTone(uint8_t ulPin) {
    if(IO_NOT_VALID(ulPin)) return;

    if(IO_NOT_PWM(ulPin)) return;

    pinMode(ulPin, DISABLE);

    //treat as PWM
    sim_clock_gate_name_t gate = PWM_GATE(PINS_PWM_INST(ulPin));
    FTM_Type *ftmBase = PWM_FTM(PINS_PWM_INST(ulPin));

    if(SIM_HAL_GetGateCmd(SIM, gate)) {
        FTM_HAL_Reset(ftmBase);
        SIM_HAL_DisableClock(SIM, gate);
    }
    cycles = 0;
}

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

void Tone_Handler(sim_clock_gate_name_t gate, FTM_Type *ftmBase, IRQn_Type irq) {
    if(cycles) cycles--;
    if(FTM_HAL_HasTimerOverflowed(ftmBase))
        FTM_HAL_ClearTimerOverflow(ftmBase);
    if(cycles == 0) {
        NVIC_DisableIRQ(irq);
        if(SIM_HAL_GetGateCmd(SIM, gate)) {
            FTM_HAL_Reset(ftmBase);
            SIM_HAL_DisableClock(SIM, gate);
        }
        if(tone_callback)
            tone_callback();
    }
}

void FTM0_IRQHandler(void) {
    Tone_Handler(kSimClockGateFtm0, FTM0, FTM0_IRQn);
}

void FTM1_IRQHandler(void) {
    Tone_Handler(kSimClockGateFtm1, FTM1, FTM1_IRQn);
}

void FTM2_IRQHandler(void) {
    Tone_Handler(kSimClockGateFtm2, FTM2, FTM2_IRQn);
}

void FTM3_IRQHandler(void) {
    Tone_Handler(kSimClockGateFtm3, FTM3, FTM3_IRQn);
}

#ifdef __cplusplus
}
#endif

void attachToneInterrupt(void (*callback)(void)) {
    tone_callback = callback;
}

void tone(uint8_t ulPin, unsigned int frequency, unsigned long duration) {

    if(frequency == 0 && duration == 0) {
        noTone(ulPin);
        return;
    }

    if(IO_NOT_VALID(ulPin)) return;

    if(IO_NOT_PWM(ulPin)) return;

    //treat as PWM
    sim_clock_gate_name_t gate = PWM_GATE(PINS_PWM_INST(ulPin));
    FTM_Type *ftmBase = PWM_FTM(PINS_PWM_INST(ulPin));
    uint32_t num_channels = PWM_CHANNELS(PINS_PWM_INST(ulPin));
    uint8_t channel = PINS_PWM_CHAN(ulPin);
    if(channel >= num_channels) return;

    //Init
    SIM_HAL_EnableClock(SIM, gate);
    FTM_HAL_Reset(ftmBase);
    for(int i = 0; i < num_channels; i++)
    {
        FTM_WR_CnSC(ftmBase, i, 0);
        FTM_WR_CnV(ftmBase, i, 0);
    }

    FTM_HAL_Init(ftmBase);

    FTM_HAL_SetSyncMode(ftmBase, kFtmUseSoftwareTrig);

    FTM_HAL_SetTofFreq(ftmBase, 0);
    FTM_HAL_SetWriteProtectionCmd(ftmBase, 0);
    FTM_HAL_SetBdmMode(ftmBase, kFtmBdmMode_00);

    uint32_t clk = SystemCoreClock;
    uint32_t mod = 0;
    int ps = 0;

    if(frequency == 0) {
        //find prescalar
        for(; ps<8; ps++) {
            clk >>= 1;
            if(clk / 1000 < 65535) {
                break;
            }
        }
        if(ps > 7) ps = 7;
        mod = (clk/1000)-1;
    } else {
        //find prescalar
        for(; ps<8; ps++) {
            clk >>= 1;
            if(frequency > clk/65535) {
                break;
            }
        }
        if(ps > 7) ps = 7;

        //find MOD
        mod = (clk/frequency)-1;
        if(mod > 65535) mod = 65535;
        if(mod < 2) mod = 2;
    }


    //SetClock
    FTM_HAL_SetClockPs(ftmBase, (ftm_clock_ps_t)ps);

    FTM_HAL_ClearTimerOverflow(ftmBase);

    if(duration > 0) {
        if(frequency == 0) {
            cycles = duration;
        } else {
            cycles = duration * frequency / 1000;
        }


        NVIC_ClearPendingIRQ(PWM_IRQ(PINS_PWM_INST(ulPin)));
        NVIC_EnableIRQ(PWM_IRQ(PINS_PWM_INST(ulPin)));

        FTM_HAL_EnableTimerOverflowInt(ftmBase);
    } else {
        cycles = 1;
    }

    FTM_HAL_SetCounterInitVal(ftmBase, 0); //not per-channel

    FTM_HAL_SetMod(ftmBase, mod);

    FTM_HAL_SetCpwms(ftmBase, 0);

    if(PINS_PORT(ulPin) != NONE)
    {
        PORT_CLOCK_ENABLE(ulPin);
        PORT_SET_MUX_PWM(ulPin);
    }

    FTM_HAL_SetChnEdgeLevel(ftmBase, channel, 2);
    FTM_HAL_SetChnMSnBAMode(ftmBase, channel, 2);
    uint16_t uCnv = 0;
    if(frequency > 0)
        uCnv = mod >> 1; //half the frequency
    else
        uCnv = 0;

    FTM_HAL_SetChnCountVal(ftmBase, channel, uCnv);
    FTM_HAL_SetClockSource(ftmBase, kClock_source_FTM_SystemClk);
    FTM_HAL_SetSoftwareTriggerCmd(ftmBase, true);
}


uint8_t digitalPinToBitMask(uint8_t pin) {
    if(IO_VALID(pin)) {
        return GPIO_PIN(pin);
    }
    return 0;
}

uint8_t digitalPinToPort(uint8_t pin) {
    if(IO_VALID(pin)) {
        return PINS_PORT(pin);
    }
    return 0;
}

volatile uint32_t* portInputRegister(uint8_t port) {
    GPIO_Type * gpio = (GPIO_Type *)(  PTA_BASE+(port*PERIPH_GPIO_SIZE));
    return (volatile uint32_t*)GPIO_PDIR_REG(gpio);
}

volatile uint32_t* portOutputRegister(uint8_t port) {
    GPIO_Type * gpio = (GPIO_Type *)(  PTA_BASE+(port*PERIPH_GPIO_SIZE));
    return (volatile uint32_t*)GPIO_PDOR_REG(gpio);
}

unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout)
{
    // cache the port and bit of the pin in order to speed up the
    // pulse width measuring loop and achieve finer resolution.  calling
    // digitalRead() instead yields much coarser resolution.
    uint8_t bit = digitalPinToBitMask(pin);
    uint8_t port = digitalPinToPort(pin);
    uint8_t stateMask = (state ? 0 : bit);
    unsigned long width = 0; // keep initialization out of time critical area

    // convert the timeout from microseconds to a number of times through
    // the initial loop; it takes 16 clock cycles per iteration.
    unsigned long numloops = 0;
    unsigned long maxloops = microsecondsToClockCycles(timeout) / 13;

    // wait for any previous pulse to end
    while ((*portInputRegister(port) & bit) == stateMask) {
        if (numloops++ == maxloops)
            return 0;
    }

    // wait for the pulse to start
    while ((*portInputRegister(port) & bit) != stateMask) {
        if (numloops++ == maxloops)
            return 0;
    }

    // wait for the pulse to stop
    while ((*portInputRegister(port) & bit) == stateMask) {
        if (numloops++ == maxloops)
            return 0;
        width++;
    }

    return clockCyclesToMicroseconds(width * 13 + 90);
}
