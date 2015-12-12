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

#define D01     (IO_MAKE_PIN(PORTD_INDEX,  7))
#define D02     (IO_MAKE_PIN(PORTD_INDEX,  4))
#define D03     (IO_MAKE_PIN(PORTD_INDEX,  6))
#define D04     (IO_MAKE_PIN(PORTC_INDEX,  8))
#define D05     (IO_MAKE_PIN(PORTC_INDEX,  1))
#define D06     (IO_MAKE_PIN(PORTC_INDEX,  2))
#define D07     (IO_MAKE_PIN(PORTC_INDEX,  0))
#define D08     (IO_MAKE_PIN(PORTC_INDEX,  5))
#define D09     (IO_MAKE_PIN(PORTE_INDEX,  0))
#define D10     (IO_MAKE_PIN(PORTE_INDEX,  1))
#define D11     (IO_MAKE_PIN(PORTD_INDEX,  1))
#define D12     (IO_MAKE_PIN(PORTD_INDEX,  2))
#define D13     (IO_MAKE_PIN(PORTD_INDEX,  5))
#define D14     (IO_MAKE_PIN(PORTD_INDEX,  3))
#define D15     (IO_MAKE_PIN(PORTB_INDEX,  1))
#define D16     (IO_MAKE_PIN(PORTA_INDEX,  5))
#define D17     (IO_MAKE_PIN(PORTA_INDEX, 12))
#define D18     (IO_MAKE_PIN(PORTA_INDEX, 13))
#define D19     (IO_MAKE_PIN(PORTB_INDEX,  0))

#define A01     (IO_MAKE_PIN_ANALOG(PORTC_INDEX, 8, 4, ANALOG_1, ANALOG_MUX_B))
#define A02     (IO_MAKE_PIN_ANALOG(PORTC_INDEX, 0, 14, ANALOG_0, ANALOG_MUX_A))
#define A03     (IO_MAKE_PIN_ANALOG(PORTE_INDEX, 0, 4, ANALOG_1, ANALOG_MUX_A))
#define A04     (IO_MAKE_PIN_ANALOG(PORTE_INDEX, 1, 5, ANALOG_1, ANALOG_MUX_A))
#define A05     (IO_MAKE_PIN_ANALOG(PORTB_INDEX, 1, 9, ANALOG_0, ANALOG_MUX_A))
#define A06     (IO_MAKE_PIN_ANALOG(PORTB_INDEX, 0, 8, ANALOG_0, ANALOG_MUX_A))
#define A07     (IO_MAKE_PIN_ANALOG(PORT_INVALID, PIN_INVALID, 23, ANALOG_0, ANALOG_MUX_A))

#define SPI0_SCK_D08    IO_MAKE_PIN_MUX(PORTC_INDEX, 5, PORT_MUX_ALT2)
#define SPI0_SCK_D11    IO_MAKE_PIN_MUX(PORTD_INDEX, 1, PORT_MUX_ALT2)
#define SPI0_SOUT_D12   IO_MAKE_PIN_MUX(PORTD_INDEX, 2, PORT_MUX_ALT2)
#define SPI0_SIN_D14    IO_MAKE_PIN_MUX(PORTD_INDEX, 3, PORT_MUX_ALT2)

#define SPI0_CS_D02     IO_MAKE_PIN_SPI_CS(PORTD_INDEX, 4, PORT_MUX_ALT2, 1)
#define SPI0_CS_D13     IO_MAKE_PIN_SPI_CS(PORTD_INDEX, 5, PORT_MUX_ALT2, 2)
#define SPI0_CS_D06     IO_MAKE_PIN_SPI_CS(PORTC_INDEX, 2, PORT_MUX_ALT2, 2)
#define SPI0_CS_D05     IO_MAKE_PIN_SPI_CS(PORTC_INDEX, 1, PORT_MUX_ALT2, 3)
#define SPI0_CS_D03     IO_MAKE_PIN_SPI_CS(PORTD_INDEX, 6, PORT_MUX_ALT2, 3)
#define SPI0_CS_U07     IO_MAKE_PIN_SPI_CS(PORTC_INDEX, 0, PORT_MUX_ALT2, 4)

#ifndef SPI_SCK
#define SPI_SCK         SPI0_SCK_D11
#endif
#define SPI_SOUT        SPI0_SOUT_D12
#define SPI_SIN         SPI0_SIN_D14
#ifndef SPI_CS
#define SPI_CS          SPI0_CS_D02
#endif

#define D02_WAKE_RISING         WAKEUP_INPUT_14_RISING
#define D02_WAKE_FALLING        WAKEUP_INPUT_14_FALLING
#define D03_WAKE_RISING         WAKEUP_INPUT_15_RISING
#define D03_WAKE_FALLING        WAKEUP_INPUT_15_FALLING
#define D05_WAKE_RISING         WAKEUP_INPUT_6_RISING
#define D05_WAKE_FALLING        WAKEUP_INPUT_6_FALLING
#define D08_WAKE_RISING         WAKEUP_INPUT_9_RISING
#define D08_WAKE_FALLING        WAKEUP_INPUT_9_FALLING
#define D10_WAKE_RISING         WAKEUP_INPUT_0_RISING
#define D10_WAKE_FALLING        WAKEUP_INPUT_0_FALLING
#define D12_WAKE_RISING         WAKEUP_INPUT_13_RISING
#define D12_WAKE_FALLING        WAKEUP_INPUT_13_FALLING
#define D18_WAKE_RISING         WAKEUP_INPUT_4_RISING
#define D18_WAKE_FALLING        WAKEUP_INPUT_4_FALLING
#define D19_WAKE_RISING         WAKEUP_INPUT_5_RISING
#define D19_WAKE_FALLING        WAKEUP_INPUT_5_FALLING

#ifdef __cplusplus
#include "Uart.h"
#include "Dash.h"

extern Uart Serial0;
extern Uart SerialCloud;
extern Uart Serial2;
extern DashClass DashPro;
#define Dash DashPro
#define Konekt SerialCloud

#endif

void initVariant(void);
