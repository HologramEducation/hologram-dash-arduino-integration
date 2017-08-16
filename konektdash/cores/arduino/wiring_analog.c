/*
  wiring_analog.c - Wiring compatibility layer analog-mode
  functions with mods for Konekt Dash and Konekt Dash Pro

  https://hologram.io

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
#include "hal/fsl_ftm_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

static int _readResolution = 10;
static adc16_resolution_t _ADCResolution = kAdc16ResolutionBitOfSingleEndAs10;
static int _writeResolution = 8;
static bool _calibrated0 = false;
static bool _calibrated1 = false;
static const uint8_t _resolution_map[] = {8,12,10,16};

int getAnalogReadResoultion(void)
{
    return _readResolution;
}

int getAnalogWriteResolution(void)
{
    return _writeResolution;
}

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

static eAnalogSampleCycles adc_cycles = AS_CYCLES_1;

void analogReadSampleCycles(eAnalogSampleCycles cycles)
{
    switch(cycles)
    {
        case AS_CYCLES_1:
        case AS_CYCLES_4:
        case AS_CYCLES_10:
        case AS_CYCLES_16:
        case AS_CYCLES_24:
            if(cycles != adc_cycles)
            {
                _calibrated0 = false;
                _calibrated1 = false;
                adc_cycles = cycles;
            }
            break;
        default:
            break;
    }
}

static adc16_long_sample_cycle_t getCycles()
{
    switch(adc_cycles)
    {
        default:
        case AS_CYCLES_24: return kAdc16LongSampleCycleOf24;
        case AS_CYCLES_4:  return kAdc16LongSampleCycleOf4;
        case AS_CYCLES_10: return kAdc16LongSampleCycleOf10;
        case AS_CYCLES_16: return kAdc16LongSampleCycleOf16;
    }
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
    ADC16_HAL_GetChnConvValue(adc, 0U);
    ADC16_HAL_SetAutoCalibrationCmd(adc, false);
    return true;
}

uint32_t analogRead( uint32_t ulPin )
{
    if(IO_NOT_ADC(ulPin)) return 0xFFFFFFFF;

    ADC_Type* adc = ADC0;
    sim_clock_gate_name_t gate_name = kSimClockGateAdc0;
    bool *calibrated = &_calibrated0;
    if(PINS_ADC_INST(ulPin) == 1)
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
        .longSampleTimeEnable = (adc_cycles != AS_CYCLES_1),
        .resolution = _ADCResolution,
        .clkSrc = kAdc16ClkSrcOfBusClk,
        .asyncClkEnable = false,
        .highSpeedEnable = false,
        .longSampleCycleMode = getCycles(),
        .hwTriggerEnable = false,
        .refVoltSrc = kAdc16RefVoltSrcOfVref,
        .continuousConvEnable = false,
        .dmaEnable = false
    };
    ADC16_HAL_ConfigConverter(adc, &config);
    if(!*calibrated)
        *calibrated = analogCalibrate(adc);

    if(PINS_PORT(ulPin) != NONE)
    {
        PORT_CLOCK_ENABLE(ulPin);
        PORT_SET_MUX_ANALOG(ulPin);
    }
    ADC16_HAL_SetChnMuxMode(adc, PINS_ADC_MUX(ulPin) == ADC_B ? kAdc16ChnMuxOfB : kAdc16ChnMuxOfA);

    adc16_chn_config_t chn_config = {
        .chnIdx = PINS_ADC_CHAN(ulPin),
        .convCompletedIntEnable = false,
        .diffConvEnable = false
    };
    ADC16_HAL_ConfigChn(adc, 0, &chn_config);
    while(!ADC16_HAL_GetChnConvCompletedFlag(adc, 0));
    uint16_t valueRead = ADC16_HAL_GetChnConvValue(adc, 0);

    return mapResolution(valueRead, _resolution_map[_ADCResolution], _readResolution);
}

#define PWM_FTM(inst)       ((inst) == PWM_3 ? FTM3 : ((inst) == PWM_2 ? FTM2 : ((inst) == PWM_1 ? FTM1 : FTM0)))
#define PWM_GATE(inst)      ((inst) == PWM_3 ? kSimClockGateFtm3 : ((inst) == PWM_2 ? kSimClockGateFtm2 : ((inst) == PWM_1 ? kSimClockGateFtm1 : kSimClockGateFtm0)))
#define PWM_CHANNELS(inst)  (((inst) == PWM_1 || (inst) == PWM_2) ? 2 : 8)

void analogWrite( uint32_t ulPin, uint32_t ulValue )
{
    if(IO_NOT_VALID(ulPin)) return;

    if(IO_NOT_PWM(ulPin))
    {
        pinMode(ulPin, OUTPUT);
        ulValue = mapResolution(ulValue, _writeResolution, 8);
        if (ulValue < 128)
            digitalWrite(ulPin, LOW);
        else
            digitalWrite(ulPin, HIGH);
    }
    else if(PINS_PWM_DAC(ulPin))
    {
        //treat as DAC
        DAC_Type *dacBase = DAC0;
        SIM_HAL_EnableClock(SIM, kSimClockGateDac0);
        uint8_t i;

        DAC_WR_SR(dacBase, 0U); /* Clear all flags. */
        DAC_WR_C0(dacBase, 0U);
        DAC_WR_C1(dacBase, 0U);
        DAC_WR_C2(dacBase, DAC_DATL_COUNT-1U);

        //Enable
        DAC_BWR_C0_DACEN(dacBase, 1U);

        uint8_t c0;

        c0 = DAC_RD_C0(dacBase);
        c0 &= ~(  DAC_C0_LPEN_MASK  );
        c0 |= DAC_C0_DACRFS_MASK;

        DAC_WR_C0(dacBase, c0);

        ulValue = mapResolution(ulValue, _writeResolution, 12);

        DAC_WR_DATL(DAC0, 0, (uint8_t)(0xFFU & ulValue) );
        DAC_BWR_DATH_DATA1(DAC0, 0, (uint8_t)((0xF00U & ulValue)>>8U) );
        DAC_BWR_C2_DACBFRP(DAC0, 0);
    }
    else
    {
        //treat as PWM
        sim_clock_gate_name_t gate = PWM_GATE(PINS_PWM_INST(ulPin));
        FTM_Type *ftmBase = PWM_FTM(PINS_PWM_INST(ulPin));
        uint32_t num_channels = PWM_CHANNELS(PINS_PWM_INST(ulPin));
        uint8_t channel = PINS_PWM_CHAN(ulPin);
        if(channel >= num_channels) return;

        if(!SIM_HAL_GetGateCmd(SIM, gate))
        {
            //Init
            SIM_HAL_EnableClock(SIM, gate);
            FTM_HAL_Reset(ftmBase);
            for(int i = 0; i < num_channels; i++)
            {
                FTM_WR_CnSC(ftmBase, i, 0);
                FTM_WR_CnV(ftmBase, i, 0);
            }

            FTM_HAL_Init(ftmBase);

            FTM_HAL_SetSyncMode(ftmBase, kFtmUseSoftwareTrig);

            FTM_HAL_SetTofFreq(ftmBase, 0);
            FTM_HAL_SetWriteProtectionCmd(ftmBase, 0);
            FTM_HAL_SetBdmMode(ftmBase, 0);

            //SetClock
            FTM_HAL_SetClockPs(ftmBase, kFtmDividedBy1);

            FTM_HAL_ClearTimerOverflow(ftmBase);

            FTM_HAL_SetCounterInitVal(ftmBase, 0); //not per-channel

            FTM_HAL_SetMod(ftmBase, SystemCoreClock / (24000u) - 1);
            FTM_HAL_SetCpwms(ftmBase, 0);
        }

        if(PINS_PORT(ulPin) != NONE)
        {
            PORT_CLOCK_ENABLE(ulPin);
            PORT_SET_MUX_PWM(ulPin);
        }

        FTM_HAL_SetChnEdgeLevel(ftmBase, channel, 2);
        FTM_HAL_SetChnMSnBAMode(ftmBase, channel, 2);

        uint16_t uMod = FTM_HAL_GetMod(ftmBase);
        uint16_t uCnv = uMod * ulValue / (1 << _writeResolution);
        /* For 100% duty cycle */
        if(uCnv >= uMod)
        {
            uCnv = uMod + 1;
        }

        FTM_HAL_SetChnCountVal(ftmBase, channel, uCnv);
        FTM_HAL_SetClockSource(ftmBase, kClock_source_FTM_SystemClk);
        FTM_HAL_SetSoftwareTriggerCmd(ftmBase, true);
    }

}

#ifdef __cplusplus
}
#endif
