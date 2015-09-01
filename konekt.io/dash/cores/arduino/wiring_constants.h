/*
  wiring_constants.h - Wiring compatibility layer constants and
  function DEFINEs with mods for Konekt Dash and Konekt Dash Pro
  
  http://konekt.io
  
  Copyright (c) 2015 Konekt, Inc.  All rights reserved.
    

  Derived from file with original copyright notice:

  Copyright (c) 2014 Arduino.  All right reserved.

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

#ifndef _WIRING_CONSTANTS_
#define _WIRING_CONSTANTS_

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

#define LOW             (0x0)
#define HIGH            (0x1)

#define INPUT           (0x0)
#define OUTPUT          (0x1)
#define INPUT_PULLUP    (0x2)
#define INPUT_PULLDOWN  (0x3)

#define PI 3.1415926535897932384626433832795
#define HALF_PI 1.5707963267948966192313216916398
#define TWO_PI 6.283185307179586476925286766559
#define DEG_TO_RAD 0.017453292519943295769236907684886
#define RAD_TO_DEG 57.295779513082320876798154814105
#define EULER 2.718281828459045235360287471352

#define SERIAL  0x0
#define DISPLAY 0x1

enum BitOrder {
	LSBFIRST = 0,
	MSBFIRST = 1
};

#define PORTA_INDEX 0
#define PORTB_INDEX 1
#define PORTC_INDEX 2
#define PORTD_INDEX 3
#define PORTE_INDEX 4

#define PORT_MUX_DISABLED   (0U)
#define PORT_MUX_GPIO       (1U)
#define PORT_MUX_ALT2       (2U)
#define PORT_MUX_ALT3       (3U)
#define PORT_MUX_ALT4       (4U)
#define PORT_MUX_ALT5       (5U)
#define PORT_MUX_ALT6       (6U)
#define PORT_MUX_ALT7       (7U)

#define PORT_PULL_DOWN      (0U)
#define PORT_PULL_UP        (1U)

#define IO_PORT_SHIFT     		(0x8U)
#define IO_MUX_SHIFT      		(0x16U)

#define IO_MAKE_PIN_MUX(r,p,m)	(((m)<< IO_MUX_SHIFT) | ((r)<< IO_PORT_SHIFT) | (p))
#define IO_MAKE_PIN(r,p)		(IO_MAKE_PIN_MUX(r,p,PORT_MUX_DISABLED))
#define IO_MUX(v)         		(((v) >> IO_MUX_SHIFT) & 0xFFU)
#define IO_PORT(v)        		(((v) >> IO_PORT_SHIFT) & 0xFFU)
#define IO_PIN(v)         		((v) & 0xFFU)
#define IO_ADD_MUX(v,m)		    (((m)<< IO_MUX_SHIFT) | (v & 0xFFFFU))

#define PORT_PORT(v)      		g_portBase[IO_PORT(v)]
#define PORT_PIN(v)       		IO_PIN(v)
#define PORT_MUX(v)       		IO_MUX(v)

#define PORT_APPLY_MUX(v)		(PORT_WR_PCR_MUX(PORT_PORT(v), PORT_PIN(v), PORT_MUX(v)))
#define PORT_SET_MUX(v, m)   	(PORT_WR_PCR_MUX(PORT_PORT(v), PORT_PIN(v), m))

#define GPIO_PORT(v)      		g_gpioBase[IO_PORT(v)]
#define GPIO_PIN(v)       		((1U) << IO_PIN(v))

#define PORT_CLOCK_ENABLE(v)	(SIM_SET_SCGC5(SIM, 0x200 << (IO_PORT(v))))


#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif /* _WIRING_CONSTANTS_ */
