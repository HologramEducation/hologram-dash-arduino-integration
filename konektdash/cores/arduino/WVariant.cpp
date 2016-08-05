/*
  WVariant.cpp

  http://hologram.io

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

#include "WVariant.h"

/*
 * Arduino |  PORT  | Dash | GPIO | AA | DA
 * --------+--------+------+------+----+----+-------------------------
 *   0     |  PTD6  |  R05 | D03  | -- | --- RX0
 *   1     |  PTD7  |  R03 | D01  | -- | --- TX0
 *   2     |  PTA12 |  L11 | D17  | -- | --- CANTX0
 *   3     |  PTD4  |  R04 | D02  | -- | --- WU14 RTS0
 *   4     |  PTD6  |  R05 | D03  | -- | --- Duplicate of 0
 *   5     |  PTB1  |  L09 | D15  | A2 | A05
 *   6     |  PTC2  |  R08 | D06  | -- | ---
 *   7     |  PTC5  |  R10 | D08  | -- | --- WU09 -SCK- (WAKE_M1)
 *   8     |  PTA13 |  L12 | D18  | -- | --- WU04 CANRX0
 *   9     |  PTA5  |  L10 | D16  | -- | ---
 *  10     |  PTD5  |  L07 | D13  | -- | ---
 *  11     |  PTD2  |  L06 | D12  | -- | --- WU13 MOSI RX2
 *  12     |  PTD3  |  L08 | D14  | -- | ---      MISO TX2
 *  13     |  PTD1  |  L05 | D11  | -- | --- SCK
 *  14     |  PTE0  |  L03 | D09  | A4 | A03 SDA
 *  15     |  PTE1  |  L04 | D10  | A5 | A04 SCL

 //Other pins
 *  16     |  PTC8  |  R06 | D04  | A0 | A01
 *  17     |  PTC1  |  R07 | D05  | -- | ---
 *  18     |  PTC0  |  R09 | D07  | A1 | A02
 *  19     |  PTB0  |  L13 | D19  | A3 | A06
 *  20     |  DAC0  |  L14 | ---  | A6 | A08

 //Internal-only pins
 *  21     |  PTC3  |  --- | ---  | -- | --- RX1 (M2_TX_TO_M1_RX)
 *  22     |  PTC4  |  --- | ---  | -- | --- TX1 (M1_TX_TO_M2_RX)
 *  23     |  PTA4  |  --- | ---  | -- | --- (EZP_CS)
 *  24     |  PTB16 |  --- | ---  | -- | --- (+3.3V_GOOD)
 *  25     |  PTB19 |  --- | ---  | -- | --- M1_LED
 *  26     |  PTB18 |  --- | ---  | -- | --- M2_RESET
 *  27     |  PTB3  |  --- | ---  | -- | --- SDA0 (BL_SDA)
 *  28     |  PTB2  |  --- | ---  | -- | --- SCL0 (BL_SCL)

//Dash Only
 *  29     |  PTC9  |  R11 | ---  | -- | --- (Dash Only) (WAKE_M2)
 *  30     |  PTC6  |  R13 | D26  | -- | --- (Dash Only)
 *  31     |  PTC7  |  R14 | D27  | -- | --- (Dash Only)
 *  32     |  PTC10 |  R15 | D28  | -- | --- (Dash Only)
 *  33     |  PTD0  |  R16 | D29  | -- | --- (Dash Only)
 *  34     |  PTC11 |  R17 | D30  | -- | --- (Dash Only)
 *  35     |  A0DM3 |  L18 | ---  | -- | A07 (Dash Only)
 *  36     |  A0DP0 |  R18 | ---  | -- | A09 (Dash Only)

//Don't map:
 *  --     |  PTA0  |  --- | ---  | -- | --- TCLK (M1_TCK_SWDCLK)
 *  --     |  PTA1  |  --- | ---  | -- | --- TDI (M1_TDI)
 *  --     |  PTA2  |  --- | ---  | -- | --- TDO (M1_TDO_SWO)
 *  --     |  PTA3  |  --- | ---  | -- | --- TMS (M1_TMS_SWDIO)
 */

extern const PinDescription g_APinDescription[]=
{
//     PORT   PIN  WAKE                     ANALOG  UART   I2C   SPI        PWM
    {PORT_D,    6,   15, ADC_MUX(ADC_0,  7, ADC_B), MUX3, NONE, NONE, PWM_MUX(PWM_0, 6, MUX4)}, // 0
    {PORT_D,    7, NONE,                      NONE, MUX3, NONE, NONE, PWM_MUX(PWM_0, 7, MUX4)}, // 1
    {PORT_A,   12, NONE,                      NONE, NONE, MUX5, NONE, PWM_MUX(PWM_1, 0, MUX3)}, // 2
    {PORT_D,    4,   14,                      NONE, MUX3, NONE, NONE, PWM_MUX(PWM_0, 4, MUX4)}, // 3
    {PORT_D,    6,   15, ADC_MUX(ADC_0,  7, ADC_B), MUX3, NONE, NONE, PWM_MUX(PWM_0, 6, MUX4)}, // 4 DUPLICATE OF 0!
    {PORT_B,    1, NONE, ADC_PIN(ADC_0,  9),        NONE, MUX2, NONE, PWM_MUX(PWM_1, 1, MUX3)}, // 5
    {PORT_C,    2, NONE, ADC_MUX(ADC_0,  4, ADC_B), MUX3, NONE, NONE, PWM_MUX(PWM_0, 1, MUX4)}, // 6
    {PORT_C,    5,    9,                      NONE, NONE, NONE, MUX2,                    NONE}, // 7
    {PORT_A,   13,    4,                      NONE, NONE, MUX5, NONE, PWM_MUX(PWM_1, 1, MUX3)}, // 8
    {PORT_A,    5, NONE,                      NONE, NONE, NONE, NONE, PWM_MUX(PWM_0, 2, MUX3)}, // 9
    {PORT_D,    5, NONE, ADC_MUX(ADC_0,  6, ADC_B), MUX3, NONE, NONE, PWM_MUX(PWM_0, 5, MUX4)}, //10
    {PORT_D,    2,   13,                      NONE, MUX3, MUX7, MUX2, PWM_MUX(PWM_3, 2, MUX4)}, //11
    {PORT_D,    3, NONE,                      NONE, MUX3, MUX7, MUX2, PWM_MUX(PWM_3, 3, MUX4)}, //12
    {PORT_D,    1, NONE, ADC_MUX(ADC_0,  5, ADC_B), MUX3, NONE, MUX2, PWM_MUX(PWM_3, 1, MUX4)}, //13
    {PORT_E,    0, NONE, ADC_MUX(ADC_1,  4, ADC_A), MUX3, MUX6, NONE,                    NONE}, //14
    {PORT_E,    1,    0, ADC_MUX(ADC_1,  5, ADC_A), MUX3, MUX6, NONE,                    NONE}, //15
//OTHER
    {PORT_C,    8, NONE, ADC_MUX(ADC_1,  4, ADC_B), NONE, NONE, NONE, PWM_MUX(PWM_3, 4, MUX3)}, //16
    {PORT_C,    1,    6, ADC_PIN(ADC_0, 15),        MUX3, NONE, NONE, PWM_MUX(PWM_0, 0, MUX4)}, //17
    {PORT_C,    0, NONE, ADC_PIN(ADC_0, 14),        NONE, NONE, NONE,                    NONE}, //18
    {PORT_B,    0,    5, ADC_PIN(ADC_0,  8),        NONE, MUX2, NONE, PWM_MUX(PWM_1, 0, MUX3)}, //19
    {  NONE, NONE, NONE, ADC_PIN(ADC_0, 23),        NONE, NONE, NONE,              PWM_DAC(0)}, //20 DAC0
//INTERNAL
    {PORT_C,    3,    7,                      NONE, MUX3, NONE, MUX2,                    NONE}, //21
    {PORT_C,    4,    8,                      NONE, MUX3, NONE, MUX2,                    NONE}, //22
    {PORT_A,    4,    3,                      NONE, NONE, NONE, NONE,                    NONE}, //23
    {PORT_B,   16, NONE,                      NONE, MUX3, NONE, NONE,                    NONE}, //24
    {PORT_B,   19, NONE,                      NONE, NONE, NONE, NONE, PWM_MUX(PWM_2, 1, MUX3)}, //25
    {PORT_B,   18, NONE,                      NONE, NONE, NONE, NONE,                    NONE}, //26
    {PORT_B,    3, NONE, ADC_PIN(ADC_0, 13),        MUX3, MUX2, NONE,                    NONE}, //27
    {PORT_B,    2, NONE, ADC_PIN(ADC_0, 12),        MUX3, MUX2, NONE,                    NONE}, //28
//DASH ONLY
    {PORT_C,    9, NONE, ADC_MUX(ADC_1,  5, ADC_B), NONE, NONE, NONE,                    NONE}, //29
    {PORT_C,    6,   10,                      NONE, NONE, NONE, MUX2,                    NONE}, //30
    {PORT_C,    7, NONE,                      NONE, NONE, NONE, MUX2,                    NONE}, //31
    {PORT_C,   10, NONE, ADC_MUX(ADC_1,  6, ADC_B), NONE, MUX2, NONE, PWM_MUX(PWM_3, 6, MUX3)}, //32
    {PORT_D,    0,   12,                      NONE, MUX3, NONE, NONE, PWM_MUX(PWM_3, 0, MUX4)}, //33
    {PORT_C,   11,   11, ADC_MUX(ADC_1,  7, ADC_B), NONE, MUX2, NONE, PWM_MUX(PWM_3, 7, MUX3)}, //34
    {  NONE, NONE, NONE, ADC_PIN(ADC_0, 21),        NONE, NONE, NONE,                    NONE}, //35
    {  NONE, NONE, NONE, ADC_PIN(ADC_0,  0),        NONE, NONE, NONE,                    NONE}, //36
};

Uart Serial0(UART0, kSimClockGateUart0, DEFAULT_SYSTEM_CLOCK, UART0_RX_TX_IRQn, 0, 1);
Uart SerialCloud(UART1, kSimClockGateUart1, DEFAULT_SYSTEM_CLOCK, UART1_RX_TX_IRQn, 21, 22);
Uart Serial2(UART2, kSimClockGateUart2, DEFAULT_BUS_CLOCK, UART2_RX_TX_IRQn, 11, 12);

TwoWire WireInternal(I2C0, kSimClockGateI2c0, DEFAULT_BUS_CLOCK, I2C0_IRQn, 27, 28);
TwoWire Wire(I2C1, kSimClockGateI2c1, DEFAULT_BUS_CLOCK, I2C1_IRQn, 14, 15);

DashClass Dash(WireInternal);

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

void UART0_RX_TX_IRQHandler(void)
{
    Serial0.IrqHandler();
    Dash.wakeFromSleep();
}

void UART1_RX_TX_IRQHandler(void)
{
    SerialCloud.IrqHandler();
    Dash.wakeFromSleep();
}

void UART2_RX_TX_IRQHandler(void)
{
    Serial2.IrqHandler();
    Dash.wakeFromSleep();
}

void PIT0_IRQHandler(void)
{
    Dash.pulseInterrupt();
}

void PIT1_IRQHandler(void)
{
    Dash.wakeFromSnooze();
}

void PORTA_IRQHandler(void)
{
    handleInterrupt(PORT_A);
    Dash.wakeFromSleep();
}

void PORTB_IRQHandler(void)
{
    handleInterrupt(PORT_B);
    Dash.wakeFromSleep();
}

void PORTC_IRQHandler(void)
{
    handleInterrupt(PORT_C);
    Dash.wakeFromSleep();
}

void PORTD_IRQHandler(void)
{
    handleInterrupt(PORT_D);
    Dash.wakeFromSleep();
}

void PORTE_IRQHandler(void)
{
    handleInterrupt(PORT_E);
    Dash.wakeFromSleep();
}

void LPTMR0_IRQHandler(void)
{
    LPTMR_SET_CSR(LPTMR0, LPTMR_CSR_TCF_MASK);
}

void I2C0_IRQHandler(void)
{
    WireInternal.onService();
}

void I2C1_IRQHandler(void)
{
    Wire.onService();
}

#define TO_HEX(v) ( ((v) > 9) ? ((v)-10+'A') : ((v)+'0') )

uint8_t USB_STR_SERIAL_NUMBER[USB_STR_SERIAL_NUMBER_SIZE + USB_STR_DESC_SIZE];

void load_sn(uint32_t src, uint8_t *dst)
{
    for(int i=0; i<16; i+=2)
        dst[i] = TO_HEX((src >> (28 - (i*2))) & 0xF);
}

void wvariant_init(void)
{
    memset(USB_STR_SERIAL_NUMBER, 0, sizeof(USB_STR_SERIAL_NUMBER));
    USB_STR_SERIAL_NUMBER[0] = sizeof(USB_STR_SERIAL_NUMBER);
    USB_STR_SERIAL_NUMBER[1] = USB_STRING_DESCRIPTOR,
    load_sn(SIM_UIDH, &USB_STR_SERIAL_NUMBER[2]);
    load_sn(SIM_UIDMH, &USB_STR_SERIAL_NUMBER[18]);
    load_sn(SIM_UIDML, &USB_STR_SERIAL_NUMBER[34]);
    load_sn(SIM_UIDL, &USB_STR_SERIAL_NUMBER[50]);

    NVIC_SetPriority(UART0_RX_TX_IRQn, 2);
    NVIC_SetPriority(UART1_RX_TX_IRQn, 1);
    NVIC_SetPriority(UART2_RX_TX_IRQn, 3);
    NVIC_SetPriority(PIT0_IRQn, 8);
    NVIC_SetPriority(PIT1_IRQn, 5);
    NVIC_SetPriority(PORTA_IRQn, 12);
    NVIC_SetPriority(PORTB_IRQn, 12);
    NVIC_SetPriority(PORTC_IRQn, 12);
    NVIC_SetPriority(PORTD_IRQn, 12);
    NVIC_SetPriority(PORTE_IRQn, 12);
}

#ifdef __cplusplus
}
#endif
