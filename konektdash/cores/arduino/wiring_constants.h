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
#define CHANGE          (0x2)
#define FALLING         (0x3)
#define RISING          (0x4)

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

typedef struct
{
    uint8_t port;
    uint8_t pin;
    uint8_t wakeup;
    uint8_t adc;
    uint8_t uartMux;
    uint8_t i2cMux;
    uint8_t spiMux;
    uint8_t pwm;
} PinDescription;

#define PORT_A                      (0U)
#define PORT_B                      (1U)
#define PORT_C                      (2U)
#define PORT_D                      (3U)
#define PORT_E                      (4U)

#define MUX_DISABLED                (0U)
#define MUX_ANALOG                  (0U)
#define MUX_GPIO                    (1U)
#define MUX2                        (2U)
#define MUX3                        (3U)
#define MUX4                        (4U)
#define MUX5                        (5U)
#define MUX6                        (6U)
#define MUX7                        (7U)

#define PORT_PULL_DOWN              (0U)
#define PORT_PULL_UP                (1U)

#define SHIFT_ADC_CHAN              (0u)
#define SHIFT_ADC_INST              (5u)
#define SHIFT_ADC_MUX               (6u)
#define MASK_ADC_CHAN               (0x1F)
#define MASK_ADC_INST               (1u)
#define MASK_ADC_MUX                (1u)

#define ADC_GET_CHAN(a)             (((a) >> SHIFT_ADC_CHAN) & MASK_ADC_CHAN)
#define ADC_GET_INST(a)             (((a) >> SHIFT_ADC_INST) & MASK_ADC_INST)
#define ADC_GET_MUX(a)              (((a) >> SHIFT_ADC_MUX) & MASK_ADC_MUX)

#define PINS_PORT(io)               (g_APinDescription[(io)].port)
#define PINS_PIN(io)                (g_APinDescription[(io)].pin)
#define PINS_WAKEUP(io)             (g_APinDescription[(io)].wakeup)
#define PINS_ADC_CHAN(io)           (ADC_GET_CHAN(g_APinDescription[(io)].adc))
#define PINS_ADC_INST(io)           (ADC_GET_INST(g_APinDescription[(io)].adc))
#define PINS_ADC_MUX(io)            (ADC_GET_MUX(g_APinDescription[(io)].adc))
#define PINS_MUX_UART(io)           (g_APinDescription[(io)].uartMux)
#define PINS_MUX_I2C(io)            (g_APinDescription[(io)].i2cMux)
#define PINS_MUX_SPI(io)            (g_APinDescription[(io)].spiMux)
#define PINS_PWM_CHAN(io)           (PWM_GET_CHAN(g_APinDescription[(io)].pwm))
#define PINS_PWM_INST(io)           (PWM_GET_INST(g_APinDescription[(io)].pwm))
#define PINS_PWM_MUX(io)            (PWM_GET_MUX(g_APinDescription[(io)].pwm))
#define PINS_MUX_PWM(io)            (PINS_PWM_MUX(io) == 0 ? MUX4 : MUX3)
#define PINS_PWM_DAC(io)            (PWM_GET_DAC(g_APinDescription[(io)].pwm))

#define PERIPH_PORT_SIZE            0x1000
#define PERIPH_FROM_PORT(port)      (PORT_Type *)(PORTA_BASE+((port)*PERIPH_PORT_SIZE))
#define PERIPH_PORT(io)             PERIPH_FROM_PORT(PINS_PORT(io)) //(PORT_Type *)(PORTA_BASE+(PINS_PORT(io)*PERIPH_PORT_SIZE)) //g_portBase[PINS_PORT(io)]

#define PERIPH_GPIO_SIZE            0x0040
#define PERIPH_GPIO(io)             (GPIO_Type *)(  PTA_BASE+(PINS_PORT(io)*PERIPH_GPIO_SIZE)) //g_gpioBase[PINS_PORT(io)]

#define PORT_SET_MUX(io, mux)       (PORT_WR_PCR_MUX(PERIPH_PORT(io), PINS_PIN(io), (mux)))
#define PORT_SET_MUX_DISABLED(io)   (PORT_SET_MUX((io), MUX_DISABLED))
#define PORT_SET_MUX_ANALOG(io)     (PORT_SET_MUX((io), MUX_ANALOG))
#define PORT_SET_MUX_GPIO(io)       (PORT_SET_MUX((io), MUX_GPIO))
#define PORT_SET_MUX_UART(io)       (PORT_SET_MUX((io), PINS_MUX_UART(io)))
#define PORT_SET_MUX_I2C(io)        (PORT_SET_MUX((io), PINS_MUX_I2C(io)))
#define PORT_SET_MUX_SPI(io)        (PORT_SET_MUX((io), PINS_MUX_SPI(io)))
#define PORT_SET_MUX_PWM(io)        (PORT_SET_MUX((io), PINS_MUX_PWM(io)))

#define GPIO_PIN(io)                ((1U) << (PINS_PIN(io)))
#define GPIO_PIN_DIRECTION(io)      (((GPIO_RD_PDDR(PERIPH_GPIO(io)) & GPIO_PIN(io)) == 0) ? (INPUT) : (OUTPUT))

#define IO_VALID(io)                ((io) <= PINS_COUNT)
#define IO_NOT_VALID(io)            ((io) > PINS_COUNT)
#define IO_NOT_DIGITAL(io)          ((IO_NOT_VALID(io)) || (g_APinDescription[(io)].port == NONE))
#define IO_NOT_ADC(io)              ((IO_NOT_VALID(io)) || (g_APinDescription[(io)].adc == NONE))
#define IO_NOT_PWM(io)              ((IO_NOT_VALID(io)) || (g_APinDescription[(io)].pwm == NONE))

#define PORT_CLOCK_ENABLE(io)       (SIM_SET_SCGC5(SIM, 0x200 << (PINS_PORT(io))))

#define NONE                        ((uint8_t)0xFF)
#define ADC_MUX(inst, chan, mux)    (((mux) << SHIFT_ADC_MUX) | ((inst) << SHIFT_ADC_INST) | ((chan) << SHIFT_ADC_CHAN))
#define ADC_PIN(inst, chan)         (ADC_MUX((inst), (chan), (0)))
#define ADC_0                       (0U)
#define ADC_1                       (1U)
#define ADC_A                       (0U)
#define ADC_B                       (1U)

#define SHIFT_PWM_CHAN              (0u)
#define SHIFT_PWM_INST              (4u)
#define SHIFT_PWM_MUX               (6u)
#define SHIFT_PWM_DAC               (7u)
#define MASK_PWM_CHAN               (15u)
#define MASK_PWM_INST               (3u)
#define MASK_PWM_MUX                (1u)
#define MASK_PWM_DAC                (1u)

#define PWM_GET_CHAN(a)             (((a) >> SHIFT_PWM_CHAN) & MASK_PWM_CHAN)
#define PWM_GET_INST(a)             (((a) >> SHIFT_PWM_INST) & MASK_PWM_INST)
#define PWM_GET_MUX(a)              (((a) >> SHIFT_PWM_MUX) & MASK_PWM_MUX)
#define PWM_GET_DAC(a)              (((a) >> SHIFT_PWM_DAC) & MASK_PWM_DAC)

#define PWM_MUX(inst, chan, mux)    (((mux & MASK_PWM_MUX) << SHIFT_PWM_MUX) | ((inst) << SHIFT_PWM_INST) | ((chan) << SHIFT_PWM_CHAN))
#define PWM_DAC(inst)               ((1 << SHIFT_PWM_DAC) | ((inst) << SHIFT_PWM_INST))
#define PWM_0                       (0u)
#define PWM_1                       (1u)
#define PWM_2                       (2u)
#define PWM_3                       (3u)

extern const PinDescription g_APinDescription[];

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif /* _WIRING_CONSTANTS_ */
