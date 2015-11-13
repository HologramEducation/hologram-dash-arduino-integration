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
    // if(ulPin > PINS_COUNT)
    //     return;

    //enable port clock
    PORT_CLOCK_ENABLE(io);

    //configure port as GPIO
    PORT_SET_MUX(io, PORT_MUX_GPIO);

    //configure gpio direction
    if(mode == OUTPUT)
        GPIO_SET_PDDR(GPIO_PORT(io), GPIO_PIN(io));
    else
    {
        bool enable_pullup = true;
        if(mode == INPUT_PULLUP)
            PORT_WR_PCR_PS(PORT_PORT(io), PORT_PIN(io), PORT_PULL_UP);
        else if(mode == INPUT_PULLDOWN)
            PORT_WR_PCR_PS(PORT_PORT(io), PORT_PIN(io), PORT_PULL_DOWN);
        else
            enable_pullup = false;

        PORT_WR_PCR_PE(PORT_PORT(io), PORT_PIN(io), enable_pullup ? 1 : 0);
        GPIO_CLR_PDDR(GPIO_PORT(io), GPIO_PIN(io));
    }
}

void digitalWrite( uint32_t io, uint32_t val )
{
    if(val == LOW)
        GPIO_WR_PCOR(GPIO_PORT(io), GPIO_PIN(io));
    else
        GPIO_WR_PSOR(GPIO_PORT(io), GPIO_PIN(io));
}

int digitalRead( uint32_t io )
{
  return ( (GPIO_RD_PDIR (GPIO_PORT(io)) >> IO_PIN(io)) & 1U );
}

void digitalToggle( uint32_t io )
{
    GPIO_WR_PTOR(GPIO_PORT(io), GPIO_PIN(io));
}

#ifdef __cplusplus
}
#endif
