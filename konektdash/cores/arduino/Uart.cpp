/*
  Uart.cpp - Implements Uart class, with mods for the
  Konekt Dash and Konekt Dash Pro family

  http://konekt.io

  Copyright (c) 2015 Konekt, Inc.  All rights reserved.


  Derived from file with original copyright notice:

  Copyright (c) 2015 Arduino LLC.  All right reserved.

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

#include "Uart.h"
#include "Arduino.h"
#include "hal/fsl_uart_hal.h"

Uart::Uart(UART_Type * instance, sim_clock_gate_name_t gate_name, uint32_t clock,
    IRQn_Type irqNumber, uint32_t rx, uint32_t tx)
{
    this->instance = instance;
    this->gate_name = gate_name;
    this->clock = clock;
    this->irqNumber = irqNumber;
    this->rx = rx;
    this->tx = tx;
}

void Uart::end()
{
    UART_HAL_Init(instance);
    NVIC_DisableIRQ(irqNumber);
    SIM_HAL_DisableClock(SIM, gate_name);
    rxBuffer.clear();
}

int Uart::available()
{
    return rxBuffer.available();
}

int Uart::peek()
{
    return rxBuffer.peek();
}

int Uart::read()
{
    return rxBuffer.read_char();
}

void Uart::begin(uint32_t baudrate)
{
    SIM_HAL_EnableClock(SIM, gate_name);

    PORT_CLOCK_ENABLE(rx);
    PORT_CLOCK_ENABLE(tx);
    PORT_SET_MUX_UART(rx);
    PORT_SET_MUX_UART(tx);

#if FSL_FEATURE_SOC_UART_COUNT
    UART_HAL_Init(instance);
    UART_HAL_SetBaudRate(instance, clock, baudrate);
    UART_HAL_SetBitCountPerChar(instance, kUart8BitsPerChar);
    UART_HAL_SetParityMode(instance, kUartParityDisabled);
#if FSL_FEATURE_UART_HAS_STOP_BIT_CONFIG_SUPPORT
    UART_HAL_SetStopBitCount(instance, kUartOneStopBit);
#endif

    UART_HAL_SetIntMode(instance, kUartIntRxDataRegFull, true);
    NVIC_EnableIRQ(irqNumber);

    UART_HAL_EnableTransmitter(instance);
    UART_HAL_EnableReceiver(instance);
#endif
}

void Uart::flush()
{
    rxBuffer.clear();
}

void Uart::waitToEmpty()
{
#if FSL_FEATURE_SOC_UART_COUNT
    if(!SIM_HAL_GetGateCmd(SIM, gate_name)) return;
    uint32_t start = millis();
    while(!(UART_RD_S1(instance) & (UART_S1_TDRE_MASK)))
    {
        if(millis() - start > 10)
            return;
    }
    while(!(UART_RD_S1(instance) & (UART_S1_TC_MASK)))
    {
        if(millis() - start > 10)
            break;
    }
#endif
}

void Uart::IrqHandler()
{
#if FSL_FEATURE_SOC_UART_COUNT
    while(UART_RD_S1_RDRF(instance))
        rxBuffer.store_char(UART_RD_D(instance));
#endif
}

size_t Uart::write(const uint8_t data)
{
#if FSL_FEATURE_SOC_UART_COUNT
    if(!SIM_HAL_GetGateCmd(SIM, gate_name)) return 0;
    uint32_t start = millis();

    while (!UART_BRD_S1_TDRE(instance))
    {
        if(millis() - start > 10)
            return 0;
    }

    UART_HAL_Putchar(instance, data);
    return 1;
#endif

    return 0;
}
