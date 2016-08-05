/*
  delay.c - Implements delay() millis() and related time functions,
  with mods for the Konekt Dash and Konekt Dash Pro family

  http://konekt.io

  Copyright (c) 2015 Konekt, Inc.  All rights reserved.


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

#include "delay.h"

#ifdef __cplusplus
extern "C" {
#endif

static volatile uint32_t systick_ = 0U;

void SysTick_Handler(void)
{
    systick_++;
}

uint32_t millis( void )
{
    return systick_;
}

uint32_t micros( void )
{
    __disable_irq();
    uint32_t ticks = SysTick->VAL;
    uint32_t ms = systick_;
    __enable_irq();
    return ms*1000 + ((SysTick->LOAD-ticks)*1000 / SysTick->LOAD);
}

void delay( uint32_t ms )
{
    if(ms == 0)
        return;

    uint32_t start = systick_;
    while(systick_ < start + ms);
}

#ifdef __cplusplus
}
#endif
