/*
  Uart.h - Implements Uart class, with mods for the 
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

#pragma once

#include "RingBuffer.h"
#include "Stream.h"
#include "fsl_device_registers.h"

#include <cstddef>

class Uart : public Stream
{
public:
    Uart(UART_Type * instance, sim_clock_gate_name_t gate_name, uint32_t clock,
        IRQn_Type irqNumber, uint32_t rx, uint32_t tx);
    void begin(unsigned long baudRate); //(8N1 only) TODO add config params
    void flush();
    void IrqHandler();
    size_t write(const uint8_t data);
    void end();
    int available();
    int peek();
    int read();
    operator bool() { return true; }
    using Print::write; // pull in write(str) and write(buf, size) from Print

protected:
    RingBuffer rxBuffer;
    UART_Type * instance;
    sim_clock_gate_name_t gate_name;
    uint32_t clock;
    IRQn_Type irqNumber;
    uint32_t rx;
    uint32_t tx;
};
