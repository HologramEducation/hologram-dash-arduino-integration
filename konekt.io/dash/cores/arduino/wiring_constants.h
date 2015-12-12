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
#define PORT_INVALID 0xFF

#define PIN_INVALID 0xFF

#define PORT_MUX_DISABLED   (0U)
#define PORT_MUX_ANALOG     (0U)
#define PORT_MUX_GPIO       (1U)
#define PORT_MUX_ALT2       (2U)
#define PORT_MUX_ALT3       (3U)
#define PORT_MUX_ALT4       (4U)
#define PORT_MUX_ALT5       (5U)
#define PORT_MUX_ALT6       (6U)
#define PORT_MUX_ALT7       (7U)

#define PORT_PULL_DOWN      (0U)
#define PORT_PULL_UP        (1U)

#define ANALOG_0            (0U)
#define ANALOG_1            (1U)

#define ANALOG_MUX_A        (0U)
#define ANALOG_MUX_B        (1U)

#define ANALOG_SET          (1U)
#define ANALOG_CLEAR        (0U)

#define IO_PORT_SHIFT       (8U)
#define IO_SPI_CS_SHIFT     (5U)
#define IO_MUX_SHIFT        (16U)
#define IO_ACHAN_SHIFT      (24U)
#define IO_AINST_SHIFT      (29U)
#define IO_AMUX_SHIFT       (30U)
#define IO_ANALOG_SHIFT     (31U)

//33222222222211111111110000000000
//10987654321098765432109876543210
//<ANALOG>.....MUX.....PRTSCS<PIN>

#define WAKEUP_INPUT_0_RISING       0x00000001
#define WAKEUP_INPUT_0_FALLING      0x00000002
#define WAKEUP_INPUT_1_RISING       0x00000004
#define WAKEUP_INPUT_1_FALLING      0x00000008
#define WAKEUP_INPUT_2_RISING       0x00000010
#define WAKEUP_INPUT_2_FALLING      0x00000020
#define WAKEUP_INPUT_3_RISING       0x00000040
#define WAKEUP_INPUT_3_FALLING      0x00000080
#define WAKEUP_INPUT_4_RISING       0x00000100
#define WAKEUP_INPUT_4_FALLING      0x00000200
#define WAKEUP_INPUT_5_RISING       0x00000400
#define WAKEUP_INPUT_5_FALLING      0x00000800
#define WAKEUP_INPUT_6_RISING       0x00001000
#define WAKEUP_INPUT_6_FALLING      0x00002000
#define WAKEUP_INPUT_7_RISING       0x00004000
#define WAKEUP_INPUT_7_FALLING      0x00008000
#define WAKEUP_INPUT_8_RISING       0x00010000
#define WAKEUP_INPUT_8_FALLING      0x00020000
#define WAKEUP_INPUT_9_RISING       0x00040000
#define WAKEUP_INPUT_9_FALLING      0x00080000
#define WAKEUP_INPUT_10_RISING      0x00100000
#define WAKEUP_INPUT_10_FALLING     0x00200000
#define WAKEUP_INPUT_11_RISING      0x00400000
#define WAKEUP_INPUT_11_FALLING     0x00800000
#define WAKEUP_INPUT_12_RISING      0x01000000
#define WAKEUP_INPUT_12_FALLING     0x02000000
#define WAKEUP_INPUT_13_RISING      0x04000000
#define WAKEUP_INPUT_13_FALLING     0x08000000
#define WAKEUP_INPUT_14_RISING      0x10000000
#define WAKEUP_INPUT_14_FALLING     0x20000000
#define WAKEUP_INPUT_15_RISING      0x40000000
#define WAKEUP_INPUT_15_FALLING     0x80000000

#define IO_MAKE_PIN_MUX(r,p,m)  (((m)<< IO_MUX_SHIFT) | ((r)<< IO_PORT_SHIFT) | (p))
#define IO_MAKE_PIN(r,p)        (IO_MAKE_PIN_MUX(r,p,PORT_MUX_DISABLED))
#define IO_MAKE_PIN_ANALOG(r,p,c,i,m)   (IO_MAKE_PIN(r,p) | (ANALOG_SET << IO_ANALOG_SHIFT) | ((c)<< IO_ACHAN_SHIFT) | ((i) << IO_AINST_SHIFT) | ((m) << IO_AMUX_SHIFT))
#define IO_ADD_ANALOG(v,c,i,m)  ((ANALOG_SET << IO_ANALOG_SHIFT) | ((c)<< IO_ACHAN_SHIFT) | ((i) << IO_AINST_SHIFT) | ((m) << IO_AMUX_SHIFT) | (v & 0xFFFFFFU))
#define IO_MUX(v)               (((v) >> IO_MUX_SHIFT) & 0x7U)
#define IO_PORT(v)              (((v) >> IO_PORT_SHIFT) & 0x7U)
#define IO_PIN(v)               ((v) & 0x1FU)
#define IO_ADD_MUX(v,m)         (((m)<< IO_MUX_SHIFT) | (v & 0xFFF8FFFFU))
#define IO_VALID(v)             (IO_PORT(v) != PORT_INVALID)
#define IO_ANALOG(v)            (((v) >> IO_ANALOG_SHIFT) & ANALOG_SET)

#define IO_INT_DISABLED         (0)
#define IO_INT_RISING           (9)
#define IO_INT_FALLING          (10)

#define PORT_PORT(v)            g_portBase[IO_PORT(v)]
#define PORT_PIN(v)             IO_PIN(v)
#define PORT_MUX(v)             IO_MUX(v)

#define PORT_APPLY_MUX(v)       (IO_VALID(v) ? (PORT_WR_PCR_MUX(PORT_PORT(v), PORT_PIN(v), PORT_MUX(v))) : 0)
#define PORT_SET_MUX(v, m)      (IO_VALID(v) ? (PORT_WR_PCR_MUX(PORT_PORT(v), PORT_PIN(v), m)) : 0)

#define GPIO_PORT(v)            g_gpioBase[IO_PORT(v)]
#define GPIO_PIN(v)             ((1U) << IO_PIN(v))

#define ANALOG_CHANNEL(v)       (((v) >> IO_ACHAN_SHIFT) & 0x1FU)
#define ANALOG_INST(v)          (((v) >> IO_AINST_SHIFT) & 0x1U)
#define ANALOG_MUX(v)           (((v) >> IO_AMUX_SHIFT) & 0x1U)

#define IO_MAKE_PIN_SPI_CS(r,p,m,s) (((s)<< IO_SPI_CS_SHIFT) | ((m)<< IO_MUX_SHIFT) | ((r)<< IO_PORT_SHIFT) | (p))
#define IO_SPI_CS(v)                (((v) >> IO_SPI_CS_SHIFT) & 0x7U)

#define PORT_CLOCK_ENABLE(v)    (IO_VALID(v) ? (SIM_SET_SCGC5(SIM, 0x200 << (IO_PORT(v)))) : 0)


#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif /* _WIRING_CONSTANTS_ */
