/*
  WVariant.h - Port type definitions for the Konekt Dash and
  Konekt Dash Pro family of products.

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

#include <stdint.h>
#include "fsl_device_registers.h"
#include "wiring_constants.h"

#define SPI_INSTANCE    SPI0
#define SPI_GATE        kSimClockGateSpi0
#define SPI_CLOCK_SRC   DEFAULT_BUS_CLOCK

#ifdef __cplusplus
extern "C" {
#endif

extern PORT_Type * const g_portBase[PORT_INSTANCE_COUNT];
extern GPIO_Type * const g_gpioBase[GPIO_INSTANCE_COUNT];
extern UART_Type * const g_uartBase[UART_INSTANCE_COUNT];
extern PIT_Type  * const g_pitBase[PIT_INSTANCE_COUNT];
// extern SPI_Type * const g_spiBase[SPI_INSTANCE_COUNT];
// extern const uint32_t g_spiFifoSize[SPI_INSTANCE_COUNT];

extern void serialEventRun(void) __attribute__((weak));

#ifdef __cplusplus
}
#endif
