/*
  variant.h
  
  http://konekt.io
    
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

#include "WVariant.h"

#define D00     (IO_MAKE_PIN(PORTD_INDEX,  6))
#define D01     (IO_MAKE_PIN(PORTD_INDEX,  7))
#define D02     (IO_MAKE_PIN(PORTC_INDEX,  1))
#define D03     (IO_MAKE_PIN(PORTD_INDEX,  4))
#define D04     (IO_MAKE_PIN(PORTC_INDEX,  0))
#define D05     (IO_MAKE_PIN(PORTD_INDEX,  5))
#define D06     (IO_MAKE_PIN(PORTA_INDEX, 13))
#define D07     (IO_MAKE_PIN(PORTC_INDEX,  8))
#define D08     (IO_MAKE_PIN(PORTB_INDEX,  1))
#define D09     (IO_MAKE_PIN(PORTA_INDEX, 12))
#define D10     (IO_MAKE_PIN(PORTC_INDEX,  2))
#define D11     (IO_MAKE_PIN(PORTD_INDEX,  3))
#define D12     (IO_MAKE_PIN(PORTA_INDEX,  5))
#define D13     (IO_MAKE_PIN(PORTD_INDEX,  1))
#define D14     (IO_MAKE_PIN(PORTD_INDEX,  2))

#define DU01    (D06)
#define DU02    (D09)
#define DU03    (IO_MAKE_PIN(PORTA_INDEX,  5))//(D12)
#define DU04    (IO_MAKE_PIN(PORTE_INDEX,  0))
#define DU05    (IO_MAKE_PIN(PORTE_INDEX,  2))
#define DU06    (IO_MAKE_PIN(PORTB_INDEX,  0))
#define DU07    (D08)
#define DU09    (D10)
#define DU10    (D02)
#define DU11    (D04)
#define DU12    (D07)
#define DU13    (D13)
#define DU14    (IO_MAKE_PIN(PORTD_INDEX,  2))//(D12)
#define DU15    (D11)
#define DU16    (D03)
#define DU17    (IO_MAKE_PIN(PORTD_INDEX,  5))
#define DU18    (IO_MAKE_PIN(PORTD_INDEX,  6))
#define DU19    (IO_MAKE_PIN(PORTD_INDEX,  7))

#ifdef __cplusplus
#include "Uart.h"
#include "Dash.h"

extern Uart Serial0;
extern Uart SerialCloud;
extern Uart Serial2;
extern DashClass DashPro;

#endif

#define CLOCK_BUS                       (24000000)

void initVariant(void);
