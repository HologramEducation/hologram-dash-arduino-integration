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

#define PINS_COUNT              (29u)

#define U01      8
#define U02      2
#define U03      9
#define U04     14
#define U05     15
#define U06     19
#define U07      5
#define U08     20
#define U09      6
#define U10     17
#define U11     18
#define U12     16
#define U13     13
#define U14     11
#define U15     12
#define U16      3
#define U17     10
#define U18      4
#define U19      1

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

//Arduino compatibility
#define A0      16
#define A1      18
#define A2       5
#define A3      19
#define A4      14
#define A5      15
#define A6      20

#define SPI_SCK             13
#define SPI_SOUT            11
#define SPI_SIN             12

#define USB_STR_PRODUCT_SIZE  (38)

void initVariant(void);
