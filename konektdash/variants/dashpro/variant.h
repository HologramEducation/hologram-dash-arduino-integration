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

#define D01      1
#define D02      3
#define D03      0
#define D04     16
#define D05     17
#define D06      6
#define D07     18
#define D08      7
#define D09     14
#define D10     15
#define D11     13
#define D12     11
#define D13     10
#define D14     12
#define D15      5
#define D16      9
#define D17      2
#define D18      8
#define D19     19

#define A01     16
#define A02     18
#define A03     14
#define A04     15
#define A05      5
#define A06     19
#define A08     32
#define A10     20

//Arduino compatibility
#define A0      16
#define A1      18
#define A2       5
#define A3      19
#define A4      14
#define A5      15
#define A6      20

#define R03      1
#define R04      3
#define R05      0
#define R06     16
#define R07     17
#define R08      6
#define R09     18
#define R10      7

#define L03     14
#define L04     15
#define L05     13
#define L06     11
#define L07     10
#define L08     12
#define L09      5
#define L10      9
#define L11      2
#define L12      8
#define L13     19
#define L14     20

#define SPI_SCK             13
#define SPI_SOUT            11
#define SPI_SIN             12

#define USB_STR_PRODUCT_SIZE  (28)

void initVariant(void);
