/*
  wiring_analog.c - Wiring compatibility layer analog-mode
  functions with mods for Konekt Dash and Konekt Dash Pro

  http://konekt.io

  Copyright (c) 2015 Konekt, Inc.  All rights reserved.


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

#include "Arduino.h"
#include "wiring_constants.h"
#include "hal/fsl_adc16_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

static int _readResolution = 10;
static adc16_resolution_t _ADCResolution = kAdc16ResolutionBitOfSingleEndAs10;
static int _writeResolution = 8;
static bool _calibrated0 = false;
static bool _calibrated1 = false;
static const uint8_t _resolution_map[] = {8,12,10,16};

void analogReadResolution( int res )
{
    _readResolution = res ;
    if (res > 12)
    {
        _ADCResolution = kAdc16ResolutionBitOfSingleEndAs16;
    }
    else if (res > 10)
    {
        _ADCResolution = kAdc16ResolutionBitOfSingleEndAs12;
    }
    else if (res > 8)
    {
        _ADCResolution = kAdc16ResolutionBitOfSingleEndAs10;
    }
    else
    {
        _ADCResolution = kAdc16ResolutionBitOfSingleEndAs8;
    }
}

void analogWriteResolution( int res )
{
    _writeResolution = res ;
}

static inline uint32_t mapResolution( uint32_t value, uint32_t from, uint32_t to )
{
    if ( from == to )
    {
        return value ;
    }

    if ( from > to )
    {
        return value >> (from-to) ;
    }
    else
    {
        return value << (to-from) ;
    }
}

/*
 * Only VREF supported
 */
void analogReference( eAnalogReference ulMode )
{
}

static bool analogCalibrate(ADC_Type* adc)
{
    ADC16_HAL_SetAutoCalibrationCmd(adc, true);
    while(!ADC16_HAL_GetChnConvCompletedFlag(adc, 0U))
    {
        if(ADC16_HAL_GetAutoCalibrationFailedFlag(adc))
        {
            ADC16_HAL_SetAutoCalibrationCmd(adc, false);
            return false;
        }
    }
    volatile uint16_t val = ADC16_HAL_GetChnConvValue(adc, 0U);
    ADC16_HAL_SetAutoCalibrationCmd(adc, false);
    return true;
}

uint32_t analogRead( uint32_t ulPin )
{
    if(!IO_ANALOG(ulPin)) return 0;

    ADC_Type* adc = ADC0;
    sim_clock_gate_name_t gate_name = kSimClockGateAdc0;
    bool *calibrated = &_calibrated0;
    if(ANALOG_INST(ulPin) == ANALOG_1)
    {
        adc = ADC1;
        gate_name = kSimClockGateAdc1;
        calibrated = &_calibrated1;
    }
    SIM_HAL_EnableClock(SIM, gate_name);

    ADC16_HAL_Init(adc);

    adc16_converter_config_t config = {
        .lowPowerEnable = true,
        .clkDividerMode = kAdc16ClkDividerOf4,
        .longSampleTimeEnable = false,
        .resolution = _ADCResolution,
        .clkSrc = kAdc16ClkSrcOfBusClk,
        .asyncClkEnable = false,
        .highSpeedEnable = false,
        .longSampleCycleMode = kAdc16LongSampleCycleOf4,
        .hwTriggerEnable = false,
        .refVoltSrc = kAdc16RefVoltSrcOfVref,
        .continuousConvEnable = false,
        .dmaEnable = false
    };
    ADC16_HAL_ConfigConverter(adc, &config);
    if(!*calibrated)
        *calibrated = analogCalibrate(adc);

    PORT_CLOCK_ENABLE(ulPin);
    PORT_SET_MUX(ulPin, PORT_MUX_ANALOG);
    ADC16_HAL_SetChnMuxMode(adc, ANALOG_MUX(ulPin) == ANALOG_MUX_B ? kAdc16ChnMuxOfB : kAdc16ChnMuxOfA);

    adc16_chn_config_t chn_config = {
        .chnIdx = ANALOG_CHANNEL(ulPin),
        .convCompletedIntEnable = false,
        .diffConvEnable = false
    };
    ADC16_HAL_ConfigChn(adc, 0, &chn_config);
    while(!ADC16_HAL_GetChnConvCompletedFlag(adc, 0));
    uint16_t valueRead = ADC16_HAL_GetChnConvValue(adc, 0);

    return mapResolution(valueRead, _resolution_map[_ADCResolution], _readResolution);
}

void analogWrite( uint32_t ulPin, uint32_t ulValue )
{
    //TODO
}

#ifdef __cplusplus
}
#endif
