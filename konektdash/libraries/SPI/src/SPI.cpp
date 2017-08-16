/*
  SPI.cpp

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

#include "SPI.h"

SPISettings::SPISettings(uint32_t clock, BitOrder bitOrder, uint8_t dataMode)
: clockFreq(clock)
{
    direction = (bitOrder == MSBFIRST) ? kDspiMsbFirst : kDspiLsbFirst;
    phase = (dspi_clock_phase_t)(dataMode & 1);
    polarity = (dspi_clock_polarity_t)((dataMode >> 1) & 1);
}

Spi::Spi(SPI_Type * instance, sim_clock_gate_name_t gate_name, uint32_t clock,
    uint32_t sin, uint32_t sout, uint32_t sck):
    instance(instance), gate_name(gate_name), clock(clock),
    sin(sin), sout(sout), sck(sck), cs(0) {}

void Spi::attachInterrupt(void) {
    // Should be enableInterrupt()
}

void Spi::detachInterrupt(void) {
    // Should be disableInterrupt()
}

void Spi::usingInterrupt(uint8_t interruptNumber) {

}

void Spi::begin()
{
    SIM_HAL_EnableClock(SIM, gate_name);

    PORT_CLOCK_ENABLE(sin);
    PORT_CLOCK_ENABLE(sout);
    PORT_CLOCK_ENABLE(sck);
    PORT_SET_MUX_SPI(sin);
    PORT_SET_MUX_SPI(sout);
    PORT_SET_MUX_SPI(sck);

    DSPI_HAL_Init(instance);
    DSPI_HAL_SetMasterSlaveMode(instance, kDspiMaster);
    DSPI_HAL_SetContinuousSckCmd(instance, false);
    DSPI_HAL_SetFifoCmd(instance, false, false);
    DSPI_HAL_SetDelay(instance, kDspiCtar1, 0, 1, kDspiPcsToSck);
    DSPI_HAL_SetDelay(instance, kDspiCtar1, 0, 4, kDspiLastSckToPcs);

    applySettings(SPISettings());
}

void Spi::end()
{
    DSPI_HAL_Init(instance);
    PORT_SET_MUX_DISABLED(sin);
    PORT_SET_MUX_DISABLED(sout);
    PORT_SET_MUX_DISABLED(sck);
    SIM_HAL_DisableClock(SIM, gate_name);
}

void Spi::applySettings()
{
    DSPI_HAL_SetBaudRate(instance, kDspiCtar1, current_settings.clockFreq, clock);
    dspi_data_format_config_t config = {8, current_settings.polarity, current_settings.phase, current_settings.direction};
    DSPI_HAL_SetDataFormat(instance, kDspiCtar1, &config);
}

void Spi::applySettings(SPISettings settings)
{
    current_settings.clockFreq = settings.clockFreq;
    current_settings.polarity = settings.polarity;
    current_settings.phase = settings.phase;
    current_settings.direction = settings.direction;
    applySettings();
}

void Spi::setBitOrder(uint8_t pin, BitOrder bitOrder)
{
    current_settings.direction = (bitOrder == MSBFIRST) ? kDspiMsbFirst : kDspiLsbFirst;
    applySettings();
}

void Spi::setDataMode(uint8_t pin, uint8_t dataMode)
{
    current_settings.polarity = (dspi_clock_polarity_t)((dataMode >> 1) & 1);
    current_settings.phase = (dspi_clock_phase_t)(dataMode & 1);
    applySettings();
}

void Spi::setClockDivider(uint8_t pin, uint8_t clockDiv)
{
    if(clockDiv == 0) return;
    //Assume Arduino 16MHz clock
    setClockFrequency(16000000/clockDiv);
}

void Spi::setClockFrequency(uint32_t clockFrequency)
{
    current_settings.clockFreq = clockFrequency;
    applySettings();
}

uint32_t Spi::beginTransaction(uint32_t chip_select, SPISettings settings)
{
    applySettings(settings);
    last_rate = beginTransaction(chip_select);
    return last_rate;
}

uint32_t Spi::beginTransaction(uint32_t chip_select)
{
    cs = chip_select;
    PORT_CLOCK_ENABLE(cs);
    digitalWrite(cs, HIGH);
    pinMode(cs, OUTPUT);
    // PORT_SET_MUX_SPI(cs); //or use GPIO??? //set based on if SPI set?
    // uint32_t cs_num = IO_SPI_CS(cs);
    // DSPI_HAL_SetPcsPolarityMode(instance, (dspi_which_pcs_config_t)(1<<cs_num), kDspiPcs_ActiveLow);
    DSPI_HAL_PresetTransferCount(instance, 0);
    DSPI_HAL_Enable(instance);
    DSPI_HAL_StartTransfer(instance);
    digitalWrite(cs, LOW);
    return last_rate;
}

void Spi::endTransaction()
{
    //Stop SPI transfer and set CS high
    //This is necessary because the DSPI peripheral will only put CS in idle
    //(high) if the last byte transferred has the CS bit cleared.
    //Since the library doesn't know the last byte until after it is already
    //sent, this workaround is necessary.

    DSPI_HAL_StopTransfer(instance);
    DSPI_HAL_Disable(instance);
    pinMode(cs, OUTPUT);
    digitalWrite(cs, HIGH);
}

uint8_t Spi::transfer(uint8_t data)
{
    // // CS Enabled
    // // CTAR 1
    // // CS pin
    // // data
    // uint32_t command = ((0x9000 | (1 << (IO_SPI_CS(cs)))) << 16) | data;
    // DSPI_HAL_WriteCmdDataMastermodeBlocking(instance, command);

    // CTAR 1
    DSPI_HAL_WriteCmdDataMastermodeBlocking(instance, 0x10000000 | data);
    return DSPI_HAL_ReadData(instance);
}

#if defined (ALT_SPI)
Spi SPI(SPI_INSTANCE, SPI_GATE, SPI_CLOCK_SRC, ALT_SPI_SIN, ALT_SPI_SOUT, ALT_SPI_SCK);
#else
Spi SPI(SPI_INSTANCE, SPI_GATE, SPI_CLOCK_SRC, SPI_SIN, SPI_SOUT, SPI_SCK);
#endif
