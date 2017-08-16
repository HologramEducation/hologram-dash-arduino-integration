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

// SPI_HAS_TRANSACTION means SPI has
//   - beginTransaction()
//   - endTransaction()
//   - usingInterrupt()
//   - SPISetting(clock, bitOrder, dataMode)
#define SPI_HAS_TRANSACTION 1

// SPI_HAS_EXTENDED_CS_PIN_HANDLING means SPI has automatic
// CS pin handling and provides the following methods:
//   - begin(pin)
//   - end(pin)
//   - setBitOrder(pin, bitorder)
//   - setDataMode(pin, datamode)
//   - setClockDivider(pin, clockdiv)
//   - transfer(pin, data, SPI_LAST/SPI_CONTINUE)
//   - beginTransaction(pin, SPISettings settings) (if transactions are available)
#define SPI_HAS_EXTENDED_CS_PIN_HANDLING 1

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

    // SPI Configuration methods
    void attachInterrupt(void);
    void detachInterrupt(void);

    void begin();
    void end();

    void usingInterrupt(uint8_t interruptNumber);
    void beginTransaction(SPISettings settings) { beginTransaction(BOARD_SPI_DEFAULT_SS, settings); }
    uint32_t beginTransaction(uint32_t chip_select);
    uint32_t beginTransaction(uint32_t chip_select, SPISettings settings);
    void endTransaction();

    void setBitOrder(uint8_t pin, BitOrder order);
    void setDataMode(uint8_t pin, uint8_t mode);
    void setClockDivider(uint8_t pin, uint8_t div);

    // These methods sets the same parameters but on default pin BOARD_SPI_DEFAULT_SS
	void setBitOrder(BitOrder order) { setBitOrder(BOARD_SPI_DEFAULT_SS, order); };
	void setDataMode(uint8_t mode) { setDataMode(BOARD_SPI_DEFAULT_SS, mode); };
	void setClockDivider(uint8_t div) { setClockDivider(BOARD_SPI_DEFAULT_SS, div); };
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

extern Spi SPI;

#endif
