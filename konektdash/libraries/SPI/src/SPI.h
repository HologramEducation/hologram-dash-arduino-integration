/*
  SPI.h

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

#ifndef _SPI_H_INCLUDED
#define _SPI_H_INCLUDED

#include "Arduino.h"
#include "hal/fsl_dspi_hal.h"

#define SPI_MODE0 0x00
#define SPI_MODE1 0x01
#define SPI_MODE2 0x02
#define SPI_MODE3 0x03

// For compatibility with sketches designed for AVR @ 16 MHz
// New programs should use SPI.beginTransaction to set the SPI clock
#define SPI_CLOCK_DIV2     2
#define SPI_CLOCK_DIV4     4
#define SPI_CLOCK_DIV8     8
#define SPI_CLOCK_DIV16   16
#define SPI_CLOCK_DIV32   32
#define SPI_CLOCK_DIV64   64
#define SPI_CLOCK_DIV128 128

class SPISettings {
public:
    SPISettings(uint32_t clock, BitOrder bitOrder, uint8_t mode);
    SPISettings() : SPISettings(1000000, MSBFIRST, SPI_MODE0){}
    uint32_t clockFreq;
    dspi_clock_polarity_t polarity;
    dspi_clock_phase_t phase;
    dspi_shift_direction_t direction;

    friend class Spi;
};

class Spi {
public:
    Spi(SPI_Type * instance, sim_clock_gate_name_t gate_name, uint32_t clock,
        uint32_t sin, uint32_t sout, uint32_t sck);

    uint8_t transfer(uint8_t data=0);
    inline void transfer(void *buf, size_t count);

    void begin();
    void end();

    uint32_t beginTransaction(uint32_t chip_select);
    uint32_t beginTransaction(uint32_t chip_select, SPISettings settings);
    void endTransaction();

    // This function is deprecated.  New applications should use
    // beginTransaction() to configure SPI settings.
    void setBitOrder(uint8_t bitOrder);
    // This function is deprecated.  New applications should use
    // beginTransaction() to configure SPI settings.
    void setDataMode(uint8_t dataMode);
    // This function is deprecated.  New applications should use
    // beginTransaction() to configure SPI settings.
    void setClockDivider(uint8_t clockDiv);
    // This function is deprecated.  New applications should use
    // beginTransaction() to configure SPI settings.
    void setClockFrequency(uint32_t clockFrequency);

private:
    SPI_Type * instance;
    sim_clock_gate_name_t gate_name;
    uint32_t clock;
    uint32_t sin;
    uint32_t sout;
    uint32_t sck;
    uint32_t cs;
    uint32_t last_rate;

    SPISettings current_settings;

    void applySettings();
    void applySettings(SPISettings settings);
};

void Spi::transfer(void *buf, size_t count)
{
    uint8_t *buffer = reinterpret_cast<uint8_t *>(buf);
    for (size_t i=0; i<count; i++)
        buffer[i] = transfer(buffer[i]);
}

#if SPI_INSTANCE_COUNT > 0
    extern Spi SPI;
#if defined (ALT_SPI)
    extern Spi AltSPI;
#endif
#endif

#endif
