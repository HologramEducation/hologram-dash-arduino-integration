/*
  WVariant.h - Port type definitions for the Konekt Dash and
  Konekt Dash Pro family of products.

  https://hologram.io

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

#include "Arduino.h"

#define DASH_LED        (25)
#define DASH_WAKE       (7)
#define LED_BUILTIN     DASH_LED

#define SPI_INSTANCE    SPI0
#define SPI_GATE        kSimClockGateSpi0
#define SPI_CLOCK_SRC   DEFAULT_BUS_CLOCK
#define Serial          SerialUSB

#define USB_STR_SERIAL_NUMBER_SIZE  (64)

#ifdef __cplusplus
#include "Uart.h"
#include "Dash.h"
#include "Max1704x.h"
#include "Charger.h"
#include "Wire.h"
#include "Clock.h"
#include "Hologram.h"
#include "MCUFlash.h"
#include "SerialCloud.h"

extern Uart Serial0;
extern Uart SerialSystem;
extern Uart Serial2;
extern DashClass Dash;
extern TwoWire Wire;
extern ClockClass Clock;
extern Max1704x FuelGauge;
extern DashCharger Charger;
extern Hologram HologramCloud;
extern MCUFlash DashFlash;
extern SerialCloudClass SerialCloud;
#define DashPro Dash

#endif

#ifdef __cplusplus
extern "C" {
#endif

extern void serialEventRun(void) __attribute__((weak));
void wvariant_init(void);

#ifdef __cplusplus
}
#endif
