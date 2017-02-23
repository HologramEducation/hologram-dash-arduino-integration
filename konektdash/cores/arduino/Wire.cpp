/*
  Wire.h - Implements TwoWire/I2C class, with mods for the
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

extern "C" {
  #include <stdlib.h>
  #include <string.h>
  #include <inttypes.h>
}

#include "Wire.h"
#include "Arduino.h"

static const uint8_t MASTER_STATE_IDLE = 0;
static const uint8_t MASTER_STATE_COMPLETE = 1;
static const uint8_t MASTER_STATE_TX_NAK = 2;
static const uint8_t MASTER_STATE_ARB_LOST = 3;
static const uint8_t MASTER_STATE_READ_READY = 4;

#define TWO_WIRE_TIMEOUT 100

// Constructors ////////////////////////////////////////////////////////////////

TwoWire::TwoWire(I2C_Type * instance, sim_clock_gate_name_t gate_name, uint32_t clock,
    IRQn_Type irqNumber, uint32_t sda, uint32_t scl) :
rxBufferIndex(0), rxBufferLength(0), txAddress(0), txBufferIndex(0), txBufferLength(0),
transmitting_master(0), instance(instance), gate_name(gate_name), clock(clock),
irqNumber(irqNumber), sda(sda), scl(scl), transmitting_slave(0), receiving_slave(0),
master_state(MASTER_STATE_IDLE), rxBufferQuantity(0), master_send_stop(1)

{
    this->instance = instance;
    this->gate_name = gate_name;
    this->clock = clock;
    this->irqNumber = irqNumber;
    this->sda = sda;
    this->scl = scl;
}

// Public Methods //////////////////////////////////////////////////////////////
void TwoWire::begin(uint8_t address)
{
    rxBufferIndex = 0;
    rxBufferLength = 0;

    txBufferIndex = 0;
    txBufferLength = 0;

    slaveBufferIndex = 0;
    slaveBufferLength = 0;

    transmitting_master = false;
    transmitting_slave = false;
    receiving_slave = false;

    SIM_HAL_EnableClock(SIM, gate_name);

    PORT_CLOCK_ENABLE(sda);
    PORT_CLOCK_ENABLE(scl);
    PORT_BWR_PCR_ODE(PERIPH_PORT(sda), PINS_PIN(sda), true); //set as open drain
    PORT_BWR_PCR_ODE(PERIPH_PORT(scl), PINS_PIN(scl), true); //set as open drain
    PORT_SET_MUX_I2C(sda);
    PORT_SET_MUX_I2C(scl);
    I2C_HAL_Init(instance);

    I2C_HAL_SetAddress7bit(instance, address);
    I2C_HAL_SetStartStopIntCmd(instance, true);
    I2C_HAL_SetIntCmd(instance, true);
    NVIC_EnableIRQ(irqNumber);
    I2C_HAL_Enable(instance);
    setClock(100000);
}

void TwoWire::reset() {
    bool enabled = SIM_HAL_GetGateCmd(SIM, gate_name);
    uint8_t address = 0;
    if(enabled) {
        address = I2C_HAL_GetAddress7bit(instance);
        end();
    }
    pinMode(sda, INPUT);
    pinMode(scl, OUTPUT);
    for(int i=0; i<9; i++) {
        digitalWrite(scl, HIGH);
        delay(5);
        digitalWrite(scl, LOW);
        delay(5);
    }
    pinMode(scl, INPUT);
    if(enabled) begin(address);
}

void TwoWire::end()
{
    NVIC_DisableIRQ(irqNumber);
    I2C_HAL_Disable(instance);
    I2C_HAL_SetAddress7bit(instance, 0);
    SIM_HAL_DisableClock(SIM, gate_name);
}

void TwoWire::setClock(uint32_t frequency)
{
    I2C_HAL_SetBaudRate(instance, clock, frequency / 1000, NULL);
}

bool TwoWire::sendAddress(uint16_t slaveAddress)
{
    master_state = MASTER_STATE_IDLE;

    I2C_HAL_SetDirMode(instance, kI2CSend);

    if(master_send_stop)
        I2C_HAL_SendStart(instance);

    I2C_HAL_WriteByte(instance, slaveAddress);

    uint32_t t0 = millis();

    while(master_state == MASTER_STATE_IDLE || master_state == MASTER_STATE_ARB_LOST)
    {
        if(millis() - t0 >= TWO_WIRE_TIMEOUT || master_state == MASTER_STATE_ARB_LOST)
        {
            I2C_HAL_SendStop(instance);
            I2C_HAL_SetDirMode(instance, kI2CReceive);
            return false; //timeout
        }
    }
    return true;
}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity, uint8_t sendStop)
{
    if(quantity == 0) return 0;

    uint32_t t0 = millis();

    while(I2C_HAL_GetStatusFlag(instance, kI2CBusBusy) && !I2C_HAL_IsMaster(instance))
    {
        if(millis() - t0 >= TWO_WIRE_TIMEOUT)
            return 0; //timeout
    }

    if(quantity > BUFFER_LENGTH){
        quantity = BUFFER_LENGTH;
    }

    rxBufferQuantity = quantity;
    rxBufferIndex = 0;
    rxBufferLength = 0;

    uint16_t slaveAddr = (address << 1U) | 1;

    if(!sendAddress(slaveAddr) || master_state != MASTER_STATE_READ_READY)
    {
        I2C_HAL_SendStop(instance);
        I2C_HAL_SetDirMode(instance, kI2CReceive);
        return 0;
    }

    master_send_stop = sendStop;

    master_state = MASTER_STATE_IDLE;

    I2C_HAL_SetDirMode(instance, kI2CReceive);

    /* Send NAK if only one byte to read. */
    if (rxBufferQuantity == 0x1U)
    {
        I2C_HAL_SendNak(instance);
    }
    else
    {
        I2C_HAL_SendAck(instance);
    }

    I2C_HAL_ReadByte(instance);

    t0 = millis();

    while(master_state == MASTER_STATE_IDLE)
    {
        if(millis() - t0 >= TWO_WIRE_TIMEOUT)
        {
            rxBufferIndex = 0;
            rxBufferLength = 0;
            I2C_HAL_SendStop(instance);
            I2C_HAL_SetDirMode(instance, kI2CReceive);
            return 0; //timeout
        }
    }

    if(master_state == MASTER_STATE_COMPLETE)
    {
        rxBufferIndex = 0;
        return rxBufferLength;
    }
    else
    {
        rxBufferIndex = 0;
        rxBufferLength = 0;
        rxBufferQuantity = 0;
        I2C_HAL_SendStop(instance);
        I2C_HAL_SetDirMode(instance, kI2CReceive);
        return 0; //timeout
    }
}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity)
{
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)true);
}

uint8_t TwoWire::requestFrom(int address, int quantity)
{
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)true);
}

uint8_t TwoWire::requestFrom(int address, int quantity, int sendStop)
{
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)sendStop);
}

void TwoWire::beginTransmission(uint8_t address)
{
  // indicate that we are transmitting
  transmitting_master = true;
  // set address of targeted slave
  txAddress = address;
  // reset tx buffer iterator vars
  txBufferIndex = 0;
  txBufferLength = 0;

  clearWriteError();
}

void TwoWire::beginTransmission(int address)
{
  beginTransmission((uint8_t)address);
}

//
//	Originally, 'endTransmission' was an f(void) function.
//	It has been modified to take one parameter indicating
//	whether or not a STOP should be performed on the bus.
//	Calling endTransmission(false) allows a sketch to
//	perform a repeated start.
//
//	WARNING: Nothing in the library keeps track of whether
//	the bus tenure has been properly ended with a STOP. It
//	is very possible to leave the bus in a hung state if
//	no call to endTransmission(true) is made. Some I2C
//	devices will behave oddly if they do not see a STOP.
//
uint8_t TwoWire::endTransmission(uint8_t sendStop)
{
    /* Set direction to send for sending of address and data. */
    uint8_t result = (uint8_t)getWriteError();
    if(result != 0)
    {
        // reset tx buffer iterator vars
        txBufferIndex = 0;
        txBufferLength = 0;
        // indicate that we are done transmitting
        transmitting_master = false;
        return 1;
    }

    uint32_t t0 = millis();

    while(I2C_HAL_GetStatusFlag(instance, kI2CBusBusy) && !I2C_HAL_IsMaster(instance))
    {
        if(millis() - t0 >= TWO_WIRE_TIMEOUT)
            return 4; //timeout
    }

    uint16_t slaveAddress;
    slaveAddress = (txAddress << 1U) & 0x00FFU;
    bool sent = sendAddress(slaveAddress);
    //tx buffer also sent by interrupt

    // reset tx buffer iterator vars
    txBufferIndex = 0;
    txBufferLength = 0;
    // indicate that we are done transmitting
    transmitting_master = false;
    clearWriteError();

    result = 4;

    if(sent) //sent without timeout
    {
        if(master_state == MASTER_STATE_COMPLETE)
        {
            result = 0;
        }
        else if(master_state == MASTER_STATE_TX_NAK) //failed
        {
            result = (txBufferIndex == 0) ? 2 : 3; //address or data fail
            sendStop = true;
        }

        if(sendStop)
        {
            I2C_HAL_SendStop(instance);
        }
    }
    I2C_HAL_SetDirMode(instance, kI2CReceive);

    return result;
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)
size_t TwoWire::write(uint8_t data)
{
    if(transmitting_slave)
    {
        if(slaveBufferLength >= BUFFER_LENGTH)
        {
            setWriteError(1);
            return 0;
        }
        slaveBuffer[slaveBufferLength++] = data;
        return 1;
    }
    else if(transmitting_master)
    {
        if(txBufferLength >= BUFFER_LENGTH)
        {
            setWriteError(1);
            return 0;
        }
        txBuffer[txBufferLength++] = data;
        return 1;
    }
    return 0;
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)
size_t TwoWire::write(const uint8_t *data, size_t quantity)
{
    size_t sent = 0;
    if(transmitting_master || transmitting_slave)
    {
        for(size_t i = 0; i < quantity; ++i)
            sent += write(data[i]);
    }
    return sent;
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int TwoWire::available(void)
{
    if(receiving_slave)
        return slaveBufferLength - slaveBufferIndex;
    return rxBufferLength - rxBufferIndex;
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int TwoWire::read(void)
{
    // get each successive byte on each call
    if(receiving_slave && (slaveBufferIndex < slaveBufferLength))
        return slaveBuffer[slaveBufferIndex++];
    else if(rxBufferIndex < rxBufferLength)
        return rxBuffer[rxBufferIndex++];

    return -1;
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int TwoWire::peek(void)
{
    if(receiving_slave && (slaveBufferIndex < slaveBufferLength))
        return slaveBuffer[slaveBufferIndex];
    else if(rxBufferIndex < rxBufferLength)
        return rxBuffer[rxBufferIndex];

    return -1;
}

void TwoWire::flush(void)
{
}

void TwoWire::onReceive(void(*function)(int))
{
    onReceiveCallback = function;
}

void TwoWire::onRequest(void(*function)(void))
{
    onRequestCallback = function;
}

void TwoWire::onService(void)
{
    //interrupt handler
    uint8_t  i2cData  = 0x00;
    bool     wasArbLost = I2C_HAL_GetStatusFlag(instance, kI2CArbitrationLost);
    bool     addressed = I2C_HAL_GetStatusFlag(instance, kI2CAddressAsSlave);
    bool     stopIntEnabled = false;

    bool     startDetected = I2C_HAL_GetStartFlag(instance);
    bool     startIntEnabled = I2C_HAL_GetStartStopIntCmd(instance);
    bool     stopDetected = I2C_HAL_GetStopFlag(instance);
    stopIntEnabled = startIntEnabled;

    /* Get current slave transfer direction */
    i2c_direction_t direction = I2C_HAL_GetDirMode(instance);

    /*--------------- Handle START, STOP or REPEAT START ------------------*/
    if (stopIntEnabled && (startDetected || stopDetected))
    {
        if(startDetected)
            I2C_HAL_ClearStartFlag(instance);
        if(stopDetected)
            I2C_HAL_ClearStopFlag(instance);
        I2C_HAL_ClearInt(instance);
        if(slaveBufferLength)
        {
            if(onReceiveCallback)
            {
                receiving_slave = true;
                onReceiveCallback(slaveBufferLength);
                receiving_slave = false;
            }
            slaveBufferIndex = 0;
            slaveBufferLength = 0;
        }
        return;
    }

    /* Clear I2C IRQ.*/
    I2C_HAL_ClearInt(instance);

    if (wasArbLost)
    {
        I2C_HAL_ClearArbitrationLost(instance);
        if (!addressed)
        {
            master_state = MASTER_STATE_ARB_LOST;
            return;
        }
    }

    if(I2C_HAL_IsMaster(instance))
    {
        if (direction == kI2CSend)
        {
            //check for NAK
            /* Check whether we got an ACK or NAK from the former byte we sent */
            if (I2C_HAL_GetStatusFlag(instance, kI2CReceivedNak))
            {
                master_state = MASTER_STATE_TX_NAK;
            }
            else if(transmitting_master)
            {
                /* Continue send if still have data. TxSize/txBuff index need
                 * increment first because one byte is already sent in order
                 * to trigger interrupt */
                 if (txBufferIndex < txBufferLength)
                 {
                     /* Transmit next byte and update buffer index */
                     I2C_HAL_WriteByte(instance, txBuffer[txBufferIndex++]);
                 }
                 else
                 {
                     /* Finish send data, send STOP, disable interrupt */
                     master_state = MASTER_STATE_COMPLETE;
                 }
             }
             else
             {
                 master_state = MASTER_STATE_READ_READY; //address sent for a read
             }
        }
        else
        {
            switch (--rxBufferQuantity)
            {
                case 0x0U:
                    /* Finish receive data, send STOP, disable interrupt */
                    master_state = MASTER_STATE_COMPLETE;
                    if(master_send_stop)
                        I2C_HAL_SendStop(instance);
                    else
                        I2C_HAL_SendStart(instance);
                    break;
                case 0x1U:
                    /* For the byte before last, we need to set NAK */
                    I2C_HAL_SendNak(instance);
                    break;
                default :
                    I2C_HAL_SendAck(instance);
                    break;
            }

            rxBuffer[rxBufferLength++] = I2C_HAL_ReadByte(instance);
        }
        return;
    }

    /*--------------- Handle Address ------------------*/
    /* Addressed only happens when receiving address. */
    if (addressed) /* Slave is addressed. */
    {
        /* Master read from Slave. Slave transmit.*/
        if (I2C_HAL_GetStatusFlag(instance, kI2CSlaveTransmit))
        {
            /* Switch to TX mode*/
            I2C_HAL_SetDirMode(instance, kI2CSend);

            transmitting_slave = true;
            slaveBufferIndex = 0;
            slaveBufferLength = 0;

            if (onRequestCallback)
                onRequestCallback(); //this needs to load the transmit buffer
            else
                // create a default 1-byte response
                write((uint8_t) 0);

        }
        else /* Master write to Slave. Slave receive.*/
        {
            /* Switch to RX mode.*/
            I2C_HAL_SetDirMode(instance, kI2CReceive);
            I2C_HAL_SendAck(instance);

            /* Read dummy character.*/
            I2C_HAL_ReadByte(instance);

            slaveBufferIndex = 0;
            slaveBufferLength = 0;
        }
    }
    /*--------------- Handle Transfer ------------------*/
    else
    {
        /* Handle transmit */
        if (direction == kI2CSend)
        {
            if (I2C_HAL_GetStatusFlag(instance, kI2CReceivedNak))
            {
                /* Switch to RX mode.*/
                I2C_HAL_SetDirMode(instance, kI2CReceive);
                /* Read dummy character to release bus */
                I2C_HAL_ReadByte(instance);

                transmitting_slave = false;
                slaveBufferIndex = 0;
                slaveBufferLength = 0;
            }
            else /* ACK from receiver.*/
            {
                transmitting_slave = true;
            }
        }
        /* Handle receive */
        else
        {
            /* Get byte from data register */
            I2C_HAL_SendAck(instance);
            i2cData = I2C_HAL_ReadByte(instance);
            if(slaveBufferLength < BUFFER_LENGTH)
                slaveBuffer[slaveBufferLength++] = i2cData;
        }
    }

    if (transmitting_slave)
    {
        /* Send byte to data register */
        if (slaveBufferIndex < slaveBufferLength)
        {
            I2C_HAL_WriteByte(instance, slaveBuffer[slaveBufferIndex++]);
            if (slaveBufferIndex >= slaveBufferLength)
            {
                slaveBufferIndex = 0;
                slaveBufferLength = 0;
                transmitting_slave = false;
            }
        }
        else
            transmitting_slave = false;
    }
}
