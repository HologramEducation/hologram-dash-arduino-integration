#include "Max1704x.h"

const uint8_t Max1704x::ADDRESS = 0x36;

void Max1704x::init(TwoWire &wire)
{
    this->wire = &wire;
}

void Max1704x::reset()
{
    wire->beginTransmission(ADDRESS);
    wire->write(0xFE);
    wire->write(0x00);
    wire->write(0x54);
    wire->endTransmission();
}

void Max1704x::quickStart()
{
    wire->beginTransmission(ADDRESS);
    wire->write(0x06);
    wire->write(0x40);
    wire->write(0x00);
    wire->endTransmission();
}

uint32_t Max1704x::mv()
{
    wire->beginTransmission(ADDRESS);
    wire->write(0x02);
    wire->endTransmission(false);
    wire->requestFrom(ADDRESS, (uint8_t)2);
    wire->endTransmission();
    while(wire->available() < 2);
    uint16_t v = wire->read() << 4;
    v |= wire->read() >> 4;
    //1.25mV per bit
    return v + (v/4);
}

uint8_t Max1704x::percentage()
{
    wire->beginTransmission(ADDRESS);
    wire->write(0x04);
    wire->endTransmission(false);
    wire->requestFrom(ADDRESS, (uint8_t)2);
    wire->endTransmission();
    while(wire->available() < 2);
    uint8_t r = wire->read();
    wire->read();
    return r;
}

uint16_t Max1704x::version()
{
    wire->beginTransmission(ADDRESS);
    wire->write(0x08);
    wire->endTransmission(false);
    wire->requestFrom(ADDRESS, (uint8_t)2);
    wire->endTransmission();
    while(wire->available() < 2);
    uint16_t v = wire->read() << 8;
    v |= wire->read();
    return v;
}

uint16_t Max1704x::config()
{
    wire->beginTransmission(ADDRESS);
    wire->write(0x0C);
    wire->endTransmission(false);
    wire->requestFrom(ADDRESS, (uint8_t)2);
    wire->endTransmission();
    while(wire->available() < 2);
    uint16_t c = wire->read() << 8;
    c |= wire->read();
    return c;
}

void Max1704x::setConfig(uint16_t config_value)
{
    wire->beginTransmission(ADDRESS);
    wire->write(0x0C);
    wire->write(config_value >> 8);
    wire->write(config_value & 0xFF);
    wire->endTransmission();
}

void Max1704x::sleep()
{
    uint16_t c = config();
    c |= 0x0080; //Set the sleep bit
    setConfig(c);
}

void Max1704x::wake()
{
    uint16_t c = config();
    c &= 0xFF7F; //Clear the sleep bit
    setConfig(c);
}

bool Max1704x::isAlerted()
{
    uint16_t c = config();
    return c & 0x0020;
}

void Max1704x::clearAlert()
{
    uint16_t c = config();
    c &= 0xFFDF;
    setConfig(c);
}

void Max1704x::setAlertThreshold(uint8_t level)
{
    if(level > 32) level = 32;
    if(level < 1) level = 1;
    level = ((~level)+1) & 0x1F; //2's complement of 1-32
    uint16_t c = config() & 0xFF80;
    c |= level; //also clears alert bit
    setConfig(c);
}
