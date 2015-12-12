/*
  variant.cpp

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

#include "variant.h"

void initVariant(void)
{
}

Uart Serial0(UART0, kSimClockGateUart0, DEFAULT_SYSTEM_CLOCK, UART0_RX_TX_IRQn,
    IO_MAKE_PIN_MUX(PORTD_INDEX,  6, PORT_MUX_ALT3),
    IO_MAKE_PIN_MUX(PORTD_INDEX,  7, PORT_MUX_ALT3));
Uart SerialCloud(UART1, kSimClockGateUart1, DEFAULT_SYSTEM_CLOCK, UART1_RX_TX_IRQn,
    IO_MAKE_PIN_MUX(PORTC_INDEX,  3, PORT_MUX_ALT3),
    IO_MAKE_PIN_MUX(PORTC_INDEX,  4, PORT_MUX_ALT3));
Uart Serial2(UART2, kSimClockGateUart2, DEFAULT_BUS_CLOCK, UART2_RX_TX_IRQn,
    IO_MAKE_PIN_MUX(PORTD_INDEX,  2, PORT_MUX_ALT3),
    IO_MAKE_PIN_MUX(PORTD_INDEX,  3, PORT_MUX_ALT3));

DashClass DashPro(IO_MAKE_PIN(PORTB_INDEX, 19),
                  IO_MAKE_PIN(PORTB_INDEX, 16),
                  IO_MAKE_PIN(PORTB_INDEX, 17),
                  IO_MAKE_PIN(PORTC_INDEX, 5),
                  WAKEUP_INPUT_9_RISING | WAKEUP_INPUT_9_FALLING);

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

void UART0_RX_TX_IRQHandler(void)
{
    Serial0.IrqHandler();
}

void UART1_RX_TX_IRQHandler(void)
{
    SerialCloud.IrqHandler();
}

void UART2_RX_TX_IRQHandler(void)
{
    Serial2.IrqHandler();
}

void PIT0_IRQHandler(void)
{
    DashPro.pulseInterrupt();
}

void PORTA_IRQHandler(void)
{
    DashPro.ioInterrupt(PORTA_INDEX);
}

void PORTB_IRQHandler(void)
{
    DashPro.ioInterrupt(PORTB_INDEX);
}

void PORTC_IRQHandler(void)
{
    DashPro.ioInterrupt(PORTC_INDEX);
}

void PORTD_IRQHandler(void)
{
    DashPro.ioInterrupt(PORTD_INDEX);
}

void PORTE_IRQHandler(void)
{
    DashPro.ioInterrupt(PORTE_INDEX);
}

#ifdef __cplusplus
}
#endif
