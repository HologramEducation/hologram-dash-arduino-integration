/*
  Wire.h - Defines TwoWire/I2C class, with mods for the
  Hologram Dash

  http://hologram.io

  Copyright (c) 2016 Konekt, Inc.  All rights reserved.

  Derived from file with original copyright notice:
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

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

  Modified 2012 by Todd Krein (todd@krein.org) to implement repeated starts
*/

#ifndef TwoWire_h
#define TwoWire_h

#include <inttypes.h>
#include "Stream.h"

#include "hal/fsl_device_registers.h"
#include "hal/fsl_i2c_hal.h"
#include "wiring_constants.h"

#define BUFFER_LENGTH 32

class TwoWire : public Stream
{
  private:
    uint8_t rxBuffer[BUFFER_LENGTH];
    uint8_t rxBufferIndex;
    uint8_t rxBufferLength;
    uint8_t rxBufferQuantity;

    uint8_t txAddress;
    uint8_t txBuffer[BUFFER_LENGTH];
    uint8_t txBufferIndex;
    uint8_t txBufferLength;

    volatile uint8_t slaveBuffer[BUFFER_LENGTH];
    volatile uint8_t slaveBufferIndex;
    volatile uint8_t slaveBufferLength;

    volatile bool transmitting_master;
    volatile bool transmitting_slave;
    volatile bool receiving_slave;
    volatile bool master_send_stop;

    volatile uint8_t master_state;

    bool sendAddress(uint16_t slaveAddress);

    I2C_Type * instance;
    sim_clock_gate_name_t gate_name;
    uint32_t clock;
    IRQn_Type irqNumber;
    uint32_t sda;
    uint32_t scl;

    void (*onRequestCallback)(void);
	void (*onReceiveCallback)(int);

  public:
    TwoWire(I2C_Type * instance, sim_clock_gate_name_t gate_name, uint32_t clock,
        IRQn_Type irqNumber, uint32_t sda, uint32_t scl);
    void begin() {begin(0);}
    void begin(uint8_t);
    void end();
    void setClock(uint32_t);
    void beginTransmission(uint8_t);
    void beginTransmission(int);
    uint8_t endTransmission(void) {return endTransmission(true);};
    uint8_t endTransmission(uint8_t);
    uint8_t requestFrom(uint8_t, uint8_t);
    uint8_t requestFrom(uint8_t, uint8_t, uint8_t);
    uint8_t requestFrom(int, int);
    uint8_t requestFrom(int, int, int);
    virtual size_t write(uint8_t);
    virtual size_t write(const uint8_t *, size_t);
    virtual int available(void);
    virtual int read(void);
    virtual int peek(void);
    virtual void flush(void);
    void onReceive(void(*)(int));
	void onRequest(void(*)(void));
    void reset();

    inline size_t write(unsigned long n) { return write((uint8_t)n); }
    inline size_t write(long n) { return write((uint8_t)n); }
    inline size_t write(unsigned int n) { return write((uint8_t)n); }
    inline size_t write(int n) { return write((uint8_t)n); }
    using Print::write;

    void onService(void);
};

#endif
