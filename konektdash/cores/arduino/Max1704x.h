#pragma once

#include "Wire.h"

class Max1704x
{
public:
    void init(TwoWire &wire);
    void reset();
    void quickStart();
    uint32_t mv();
    uint8_t percentage();
    uint16_t version();
    uint16_t config();
    void setConfig(uint16_t config_value);
    void sleep();
    void wake();
    void setAlertThreshold(uint8_t level=4); //1-32%
    bool isAlerted();
    void clearAlert();

    static const uint8_t ADDRESS;

protected:
    TwoWire *wire;

};
