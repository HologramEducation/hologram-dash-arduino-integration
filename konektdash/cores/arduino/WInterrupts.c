/*
  WInterrupts.c - Interrupt handling for Konekt Dash and Konekt Dash Pro

  http://konekt.io

  Copyright (c) 2015 Konekt, Inc.  All rights reserved.


  Derived from file with original copyright notice:

  Copyright (c) 2011-2012 Arduino.  All right reserved.

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

#include "WInterrupts.h"

typedef void (*interruptCB)(void);

static interruptCB callbacks[5][20];

uint32_t digitalPinToInterrupt(uint32_t pin)
{
    return pin;
}

void attachInterrupt(uint32_t pin, void (*callback)(void), uint32_t mode)
{
    uint32_t config = 0;

    if(IO_NOT_DIGITAL(pin)) return;
    PORT_CLOCK_ENABLE(pin);

    switch(mode)
    {
        case LOW:     config = 0x8; break;
        case HIGH:    config = 0xC; break;
        default:
        case CHANGE:  config = 0xB; break;
        case FALLING: config = 0xA; break;
        case RISING:  config = 0x9; break;
    }

    callbacks[PINS_PORT(pin)][PINS_PIN(pin)] = callback;
    PORT_WR_PCR_ISF(PERIPH_PORT(pin), PINS_PIN(pin), 1);
    PORT_WR_PCR_IRQC(PERIPH_PORT(pin), PINS_PIN(pin), config);
    NVIC_EnableIRQ((IRQn_Type)(PORTA_IRQn+PINS_PORT(pin)));
}

void detachInterrupt(uint32_t pin)
{
    if(IO_NOT_DIGITAL(pin)) return;
    PORT_WR_PCR_IRQC(PERIPH_PORT(pin), PINS_PIN(pin), 0);
    PORT_WR_PCR_ISF(PERIPH_PORT(pin), PINS_PIN(pin), 1);
}

void handleInterrupt(uint32_t port)
{
    if(port > 4) return;

    uint32_t flags = PORT_RD_ISFR(PERIPH_FROM_PORT(port));
    PORT_WR_ISFR(PERIPH_FROM_PORT(port), ~0U);

    if(flags)
    {
        for(int i=0; i<20; i++)
        {
            if(flags & (1<<i))
            {
                if(callbacks[port][i])
                    callbacks[port][i]();
            }
        }
    }
}
