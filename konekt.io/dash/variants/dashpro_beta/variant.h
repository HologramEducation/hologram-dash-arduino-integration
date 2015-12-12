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

#define U01     (IO_MAKE_PIN(PORTA_INDEX, 13))
#define U02     (IO_MAKE_PIN(PORTA_INDEX, 12))
#define U03     (IO_MAKE_PIN(PORTA_INDEX,  5))
#define U04     (IO_MAKE_PIN_ANALOG(PORTE_INDEX, 0, 4, ANALOG_1, ANALOG_MUX_A))
#define U05     (IO_MAKE_PIN_ANALOG(PORTE_INDEX, 1, 5, ANALOG_1, ANALOG_MUX_A))
#define U06     (IO_MAKE_PIN_ANALOG(PORTB_INDEX, 0, 8, ANALOG_0, ANALOG_MUX_A))
#define U07     (IO_MAKE_PIN_ANALOG(PORTB_INDEX, 1, 9, ANALOG_0, ANALOG_MUX_A))
#define U08     (IO_MAKE_PIN_ANALOG(PORT_INVALID, PIN_INVALID, 23, ANALOG_0, ANALOG_MUX_A))
#define U09     (IO_MAKE_PIN_ANALOG(PORTC_INDEX, 2, 4, ANALOG_0, ANALOG_MUX_B))
#define U10     (IO_MAKE_PIN_ANALOG(PORTC_INDEX, 1, 15, ANALOG_0, ANALOG_MUX_A))
#define U11     (IO_MAKE_PIN_ANALOG(PORTC_INDEX, 0, 14, ANALOG_0, ANALOG_MUX_A))
#define U12     (IO_MAKE_PIN_ANALOG(PORTC_INDEX, 8, 4, ANALOG_1, ANALOG_MUX_B))
#define U13     (IO_MAKE_PIN_ANALOG(PORTD_INDEX, 1, 5, ANALOG_0, ANALOG_MUX_B))
#define U14     (IO_MAKE_PIN(PORTD_INDEX,  2))
#define U15     (IO_MAKE_PIN(PORTD_INDEX,  3))
#define U16     (IO_MAKE_PIN(PORTD_INDEX,  4))
#define U17     (IO_MAKE_PIN_ANALOG(PORTD_INDEX, 5, 6, ANALOG_0, ANALOG_MUX_B))
#define U18     (IO_MAKE_PIN_ANALOG(PORTD_INDEX, 6, 7, ANALOG_0, ANALOG_MUX_B))
#define U19     (IO_MAKE_PIN(PORTD_INDEX,  7))

#define D00     (U18)
#define D01     (U19)
#define D02     (U10)
#define D03     (U16)
#define D04     (U11)
#define D05     (U17)
#define D06     (U01)
#define D07     (U12)
#define D08     (U07)
#define D09     (U02)
#define D10     (U09)
#define D11     (U15)
#define D12     (U03)
#define D13     (U13)
#define D14     (U14)

#define A00      (U08)
#define A01      (U09)
#define A02      (U06)
#define A03      (U07)
#define A04      (U04)
#define A05      (U05)
#define A06      (U06)

#define SPI0_SCK_U13    IO_MAKE_PIN_MUX(PORTD_INDEX, 1, PORT_MUX_ALT2)
#define SPI0_SOUT_U14   IO_MAKE_PIN_MUX(PORTD_INDEX, 2, PORT_MUX_ALT2)
#define SPI0_SIN_U15    IO_MAKE_PIN_MUX(PORTD_INDEX, 3, PORT_MUX_ALT2)

#define SPI0_CS_U16     IO_MAKE_PIN_SPI_CS(PORTD_INDEX, 4, PORT_MUX_ALT2, 1)
#define SPI0_CS_U17     IO_MAKE_PIN_SPI_CS(PORTD_INDEX, 5, PORT_MUX_ALT2, 2)
#define SPI0_CS_U09     IO_MAKE_PIN_SPI_CS(PORTC_INDEX, 2, PORT_MUX_ALT2, 2)
#define SPI0_CS_U10     IO_MAKE_PIN_SPI_CS(PORTC_INDEX, 1, PORT_MUX_ALT2, 3)
#define SPI0_CS_U18     IO_MAKE_PIN_SPI_CS(PORTD_INDEX, 6, PORT_MUX_ALT2, 3)
#define SPI0_CS_U11     IO_MAKE_PIN_SPI_CS(PORTC_INDEX, 0, PORT_MUX_ALT2, 4)

#define SPI_SCK         SPI0_SCK_U13
#define SPI_SOUT        SPI0_SOUT_U14
#define SPI_SIN         SPI0_SIN_U15
#ifndef SPI_CS
#define SPI_CS          SPI0_CS_U16
#endif

#define U01_WAKE_RISING         WAKEUP_INPUT_4_RISING
#define U01_WAKE_FALLING        WAKEUP_INPUT_4_FALLING
#define U05_WAKE_RISING         WAKEUP_INPUT_0_RISING
#define U05_WAKE_FALLING        WAKEUP_INPUT_0_FALLING
#define U06_WAKE_RISING         WAKEUP_INPUT_5_RISING
#define U06_WAKE_FALLING        WAKEUP_INPUT_5_FALLING
#define U10_WAKE_RISING         WAKEUP_INPUT_6_RISING
#define U10_WAKE_FALLING        WAKEUP_INPUT_6_FALLING
#define U14_WAKE_RISING         WAKEUP_INPUT_13_RISING
#define U14_WAKE_FALLING        WAKEUP_INPUT_13_FALLING
#define U16_WAKE_RISING         WAKEUP_INPUT_14_RISING
#define U16_WAKE_FALLING        WAKEUP_INPUT_14_FALLING
#define U18_WAKE_RISING         WAKEUP_INPUT_15_RISING
#define U18_WAKE_FALLING        WAKEUP_INPUT_15_FALLING

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
