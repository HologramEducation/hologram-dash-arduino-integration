/*
  wiring_digital.c - Wiring compatibility layer digital-mode
  functions with mods for Konekt Dash and Konekt Dash Pro

  http://konekt.io

  Copyright (c) 2015 Konekt, Inc.  All rights reserved.


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

#include "Arduino.h"

#ifdef __cplusplus
 extern "C" {
#endif

void pinMode( uint32_t io, uint32_t mode )
{
    //if(!IO_VALID(io)) return;
    if(IO_NOT_DIGITAL(io)) return;

    //enable port clock
    PORT_CLOCK_ENABLE(io);

    //configure port as GPIO
    PORT_SET_MUX(io, MUX_GPIO);

    //configure gpio direction
    if(mode == OUTPUT)
        GPIO_SET_PDDR(PERIPH_GPIO(io), GPIO_PIN(io)); //GPIO_SET_PDDR(GPIO_PORT(io), GPIO_PIN(io));
    else
    {
        bool enable_pullup = true;
        if(mode == INPUT_PULLUP)
            PORT_WR_PCR_PS(PERIPH_PORT(io), PINS_PIN(io), PORT_PULL_UP);//PORT_WR_PCR_PS(PORT_PORT(io), PORT_PIN(io), PORT_PULL_UP);
        else if(mode == INPUT_PULLDOWN)
            PORT_WR_PCR_PS(PERIPH_PORT(io), PINS_PIN(io), PORT_PULL_DOWN);//PORT_WR_PCR_PS(PORT_PORT(io), PORT_PIN(io), PORT_PULL_DOWN);
        else
            enable_pullup = false;

        PORT_WR_PCR_PE(PERIPH_PORT(io), PINS_PIN(io), enable_pullup ? 1 : 0);//PORT_WR_PCR_PE(PORT_PORT(io), PORT_PIN(io), enable_pullup ? 1 : 0);
        GPIO_CLR_PDDR(PERIPH_GPIO(io), GPIO_PIN(io));//GPIO_CLR_PDDR(GPIO_PORT(io), GPIO_PIN(io));
    }
}

void digitalWrite( uint32_t io, uint32_t val )
{
    if(IO_NOT_DIGITAL(io)) return;

    if(val == LOW)
        GPIO_WR_PCOR(PERIPH_GPIO(io), GPIO_PIN(io));
    else
        GPIO_WR_PSOR(PERIPH_GPIO(io), GPIO_PIN(io));
}

int digitalRead( uint32_t io )
{
    if(IO_NOT_DIGITAL(io)) return 0;
    return ((GPIO_RD_PDIR(PERIPH_GPIO(io)) >> PINS_PIN(io)) & 1U);
}

void digitalToggle( uint32_t io )
{
    if(IO_NOT_DIGITAL(io)) return;
    GPIO_WR_PTOR(PERIPH_GPIO(io), GPIO_PIN(io));
}

#ifdef __cplusplus
}
#endif
