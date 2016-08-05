/*
  SerialCDC.cpp - Implements SerialCDC class, which provides a USB serial port
  for Konekt Dash and Konekt Dash Pro family

  http://konekt.io

  Copyright (c) 2015 Konekt, Inc.  All rights reserved.


  Derived from file with original copyright notice:

  Copyright (c) 2015 Arduino LLC.  All right reserved.

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

#include "SerialCDC.h"
#include "Arduino.h"

SerialCDC SerialUSB;

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

extern usb_desc_request_notify_struct_t desc_callback;
extern uint8_t USB_Desc_Set_Speed(uint32_t handle, uint16_t speed);
cdc_handle_t g_app_handle;

/* Implementation Specific Macros */
#define LINE_CODING_SIZE              (0x07)
#define COMM_FEATURE_DATA_SIZE        (0x02)

#define LINE_CODE_DTERATE_IFACE      (115200) /*e.g 9600 is 0x00002580 */
#define LINE_CODE_CHARFORMAT_IFACE   (0x00)   /* 1 stop bit */
#define LINE_CODE_PARITYTYPE_IFACE   (0x00)  /* No Parity */
#define LINE_CODE_DATABITS_IFACE     (0x08)  /* Data Bits Format */

#define STATUS_ABSTRACT_STATE_IFACE  (0x0000) /* Disable Multiplexing ENDP in
                                                  this interface will continue
                                                  to accept/offer data*/
#define COUNTRY_SETTING_IFACE        (0x0000) /* Country Code in the format as
                                                  defined in [ISO3166]-
                                                  - PLEASE CHECK THESE VALUES*/

uint8_t g_line_coding[LINE_CODING_SIZE] =
{
    /*e.g. 0x00,0x10,0x0E,0x00 : 0x000E1000 is 921600 bits per second */
    (LINE_CODE_DTERATE_IFACE >> 0) & 0x000000FF,
    (LINE_CODE_DTERATE_IFACE >> 8) & 0x000000FF,
    (LINE_CODE_DTERATE_IFACE >> 16) & 0x000000FF,
    (LINE_CODE_DTERATE_IFACE >> 24) & 0x000000FF,
    LINE_CODE_CHARFORMAT_IFACE,
    LINE_CODE_PARITYTYPE_IFACE,
    LINE_CODE_DATABITS_IFACE
};

uint8_t g_abstract_state[COMM_FEATURE_DATA_SIZE] =
{
    (STATUS_ABSTRACT_STATE_IFACE >> 0) & 0x00FF,
    (STATUS_ABSTRACT_STATE_IFACE >> 8) & 0x00FF
};

uint8_t g_country_code[COMM_FEATURE_DATA_SIZE] =
{
    (COUNTRY_SETTING_IFACE >> 0) & 0x00FF,
    (COUNTRY_SETTING_IFACE >> 8) & 0x00FF
};

static volatile bool start_app = FALSE;
static volatile bool start_transactions = FALSE;
static volatile bool g_send_busy = FALSE;
static uint8_t g_curr_recv_buf[DATA_BUFF_SIZE];
static uint8_t g_curr_trans_buf[DATA_BUFF_SIZE];

static uint16_t g_cdc_device_speed;
static uint16_t g_bulk_out_max_packet_size;
static uint16_t g_bulk_in_max_packet_size;

uint8_t USB_Get_Line_Coding(uint32_t handle,
    uint8_t interface,
    uint8_t * *coding_data)
{
    //UNUSED_ARGUMENT(handle)
    /* if interface valid */
    if (interface < USB_MAX_SUPPORTED_INTERFACES)
    {
        /* get line coding data*/
        *coding_data = g_line_coding;
        return USB_OK;
    }

    return USBERR_INVALID_REQ_TYPE;
}


uint8_t USB_Set_Line_Coding(uint32_t handle,
    uint8_t interface,
    uint8_t * *coding_data)
{
    uint8_t count;

    //UNUSED_ARGUMENT(handle)

    /* if interface valid */
    if (interface < USB_MAX_SUPPORTED_INTERFACES)
    {
        /* set line coding data*/
        for (count = 0; count < LINE_CODING_SIZE; count++)
        {
            g_line_coding[count] = *((*coding_data + USB_SETUP_PKT_SIZE) + count);
        }
        return USB_OK;
    }

    return USBERR_INVALID_REQ_TYPE;
}

uint8_t USB_Get_Abstract_State(uint32_t handle,
    uint8_t interface,
    uint8_t * *feature_data)
{
    //UNUSED_ARGUMENT(handle)
    /* if interface valid */
    if (interface < USB_MAX_SUPPORTED_INTERFACES)
    {
        /* get line coding data*/
        *feature_data = g_abstract_state;
        return USB_OK;
    }

    return USBERR_INVALID_REQ_TYPE;
}

uint8_t USB_Get_Country_Setting(uint32_t handle,
    uint8_t interface,
    uint8_t * *feature_data)
{
    //UNUSED_ARGUMENT(handle)
    /* if interface valid */
    if (interface < USB_MAX_SUPPORTED_INTERFACES)
    {
        /* get line coding data*/
        *feature_data = g_country_code;
        return USB_OK;
    }

    return USBERR_INVALID_REQ_TYPE;
}

uint8_t USB_Set_Abstract_State(uint32_t handle,
    uint8_t interface,
    uint8_t * *feature_data)
{
    uint8_t count;
    //UNUSED_ARGUMENT(handle)
    /* if interface valid */
    if (interface < USB_MAX_SUPPORTED_INTERFACES)
    {
        /* set Abstract State Feature*/
        for (count = 0; count < COMM_FEATURE_DATA_SIZE; count++)
        {
            g_abstract_state[count] = *(*feature_data + count);
        }
        return USB_OK;
    }

    return USBERR_INVALID_REQ_TYPE;
}

uint8_t USB_Set_Country_Setting(uint32_t handle,
    uint8_t interface,
    uint8_t * *feature_data)
{
    uint8_t count;
    //UNUSED_ARGUMENT (handle)

    /* if interface valid */
    if (interface < USB_MAX_SUPPORTED_INTERFACES)
    {
        for (count = 0; count < COMM_FEATURE_DATA_SIZE; count++)
        {
            g_country_code[count] = *(*feature_data + count);
        }
        return USB_OK;
    }

    return USBERR_INVALID_REQ_TYPE;
}

static void CDC_Device_Callback(uint8_t event_type, void* val, void* arg)
{
    uint32_t handle;
    handle = *((uint32_t *) arg);
    if (event_type == USB_DEV_EVENT_BUS_RESET)
    {
        start_app = FALSE;
        if (USB_OK == USB_Class_CDC_Get_Speed(handle, &g_cdc_device_speed))
        {
            USB_Desc_Set_Speed(handle, g_cdc_device_speed);
            if (USB_SPEED_HIGH == g_cdc_device_speed)
            {
                g_bulk_out_max_packet_size = HS_DIC_BULK_OUT_ENDP_PACKET_SIZE;
                g_bulk_in_max_packet_size = HS_DIC_BULK_IN_ENDP_PACKET_SIZE;
            }
            else
            {
                g_bulk_out_max_packet_size = FS_DIC_BULK_OUT_ENDP_PACKET_SIZE;
                g_bulk_in_max_packet_size = FS_DIC_BULK_IN_ENDP_PACKET_SIZE;
            }
        }
    }
    else if (event_type == USB_DEV_EVENT_CONFIG_CHANGED)
    {
        /* Schedule buffer for receive */
        USB_Class_CDC_Recv_Data(handle, DIC_BULK_OUT_ENDPOINT, g_curr_recv_buf, g_bulk_out_max_packet_size);
        start_app = TRUE;
    }
    else if (event_type == USB_DEV_EVENT_ERROR)
    {
        /* add user code for error handling */
    }
    return;
}

static uint8_t CDC_Class_Callback
(
    uint8_t event,
    uint16_t value,
    uint8_t ** data,
    uint32_t* size,
    void* arg
)
{
    cdc_handle_t handle;
    uint8_t error = USB_OK;
    handle = *((cdc_handle_t *) arg);
    switch(event)
    {
    case GET_LINE_CODING:
        error = USB_Get_Line_Coding(handle, value, data);
        break;
    case GET_ABSTRACT_STATE:
        error = USB_Get_Abstract_State(handle, value, data);
        break;
    case GET_COUNTRY_SETTING:
        error = USB_Get_Country_Setting(handle, value, data);
        break;
    case SET_LINE_CODING:
        error = USB_Set_Line_Coding(handle, value, data);
        break;
    case SET_ABSTRACT_STATE:
        error = USB_Set_Abstract_State(handle, value, data);
        break;
    case SET_COUNTRY_SETTING:
        error = USB_Set_Country_Setting(handle, value, data);
        break;
    case USB_APP_CDC_DTE_ACTIVATED:
        if (start_app == TRUE)
        {
            start_transactions = TRUE;
        }
        break;
    case USB_APP_CDC_DTE_DEACTIVATED:
        if (start_app == TRUE)
        {
            start_transactions = FALSE;
        }
        break;
    case USB_DEV_EVENT_DATA_RECEIVED:
        if ((start_app == TRUE) && (start_transactions == TRUE))
        {
            for(uint32_t i=0; i<*size; i++)
                SerialUSB.fill(g_curr_recv_buf[i]);
            Dash.wakeFromSleep();
            /* Schedule buffer for next receive event */
            USB_Class_CDC_Recv_Data(handle, DIC_BULK_OUT_ENDPOINT, g_curr_recv_buf, g_bulk_out_max_packet_size);
        }
        break;
    case USB_DEV_EVENT_SEND_COMPLETE:
        if ((start_app == TRUE) && (start_transactions == TRUE))
        {
            g_send_busy = false;
        }
        break;
    case USB_APP_CDC_SERIAL_STATE_NOTIF:
        {
        /* User: add your own code for serial_state notify event */
    }
        break;
    default:
        {
        error = USBERR_INVALID_REQ_TYPE;
        break;
    }

    }

    return error;
}

static void CDC_init(void)
{
    typedef struct
    {
        __IO uint32_t CESR; /**< Watchdog Status and Control Register High, offset: 0x0 */
    } MPU_Type; //, *MPU_MemMapPtr;
#define MPU_CESR_REG(base)                   	((base)->CESR)
#define MPU_BASE                                (0x4000D000u)
#define MPU                                     ((MPU_Type *)MPU_BASE)
#define MPU_BASE_PTR                            (MPU)
#define MPU_BASE_ADDRS                          { MPU_BASE }
#define MPU_BASE_PTRS                           { MPU }

    //disable MPU
    MPU->CESR = 0xF8000000;

    /* FMC_PFAPR: M4PFD=1,M2PFD=0,M1PFD=0,M0PFD=0,M4AP=3,M2AP=3,M1AP=3,M0AP=3 */
    #define FMC_PFAPR_VALUE      0x0010033FU
    #define FMC_PFAPR_MASK       0x0017033FU
    /* FMC_PFB0CR: B0RWSC=3,CLCK_WAY=0,CINV_WAY=0,S_B_INV=0,B0MW=1,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,CRC=0,B0DCE=1,B0ICE=1,B0DPE=1,B0IPE=1,B0SEBE=1 */
    #define FMC_PFB0CR_VALUE     0x3002001FU
    /* FMC_PFB1CR: B1RWSC=3,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,B1MW=1,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,B1DCE=1,B1ICE=1,B1DPE=1,B1IPE=1,B1SEBE=1 */
    #define FMC_PFB1CR_VALUE     0x3002001FU

    FMC_PFAPR = (FMC_PFAPR & (~FMC_PFAPR_MASK)) | FMC_PFAPR_VALUE;
    FMC_PFB0CR = FMC_PFB0CR_VALUE;
    FMC_PFB1CR = FMC_PFB1CR_VALUE;

    cdc_config_struct_t cdc_config;
    cdc_config.cdc_application_callback.callback = CDC_Device_Callback;
    cdc_config.cdc_application_callback.arg = &g_app_handle;
    cdc_config.vendor_req_callback.callback = NULL;
    cdc_config.vendor_req_callback.arg = NULL;
    cdc_config.class_specific_callback.callback = CDC_Class_Callback;
    cdc_config.class_specific_callback.arg = &g_app_handle;
    cdc_config.desc_callback_ptr = &desc_callback;
    /* Always happen in control endpoint hence hard coded in Class layer*/

    g_cdc_device_speed = USB_SPEED_FULL;
    g_bulk_out_max_packet_size = FS_DIC_BULK_OUT_ENDP_PACKET_SIZE;
    g_bulk_in_max_packet_size = FS_DIC_BULK_IN_ENDP_PACKET_SIZE;
    /* Initialize the USB interface */
    USB_Class_CDC_Init(CONTROLLER_ID, &cdc_config, &g_app_handle);
}

#ifdef __cplusplus
}
#endif

SerialCDC::SerialCDC()
:isReady(false){}

int SerialCDC::available()
{
    return rxBuffer.available();
}

int SerialCDC::peek()
{
    return rxBuffer.peek();
}

int SerialCDC::read()
{
    return rxBuffer.read_char();
}

void SerialCDC::begin(uint32_t baudrate)
{
    rxBuffer.clear();
    g_send_busy = FALSE;
    start_app = FALSE;
    start_transactions = FALSE;
    CDC_init();
    isReady = true;
}

void SerialCDC::end()
{
    USB_Class_CDC_Deinit(g_app_handle);
    isReady = false;
}

bool SerialCDC::ready()
{
    return isReady;
}

void SerialCDC::flush()
{
    rxBuffer.clear();
}

void SerialCDC::waitToEmpty()
{
    uint32_t start = millis();
    while (g_send_busy)
    {
        if(millis() - start > 500)
            break;
    }
}

void SerialCDC::fill(uint8_t data)
{
    rxBuffer.store_char(data);
}

size_t SerialCDC::write(uint8_t data)
{
    return write(&data, 1);
}

size_t SerialCDC::write(const uint8_t *buffer, size_t size)
{
    uint32_t sent = 0;
    if(!start_app || !start_transactions) return 0;

    while(sent < size)
    {
        uint32_t tx = min(size-sent, (uint32_t)(g_bulk_out_max_packet_size-1));
        uint32_t start = millis();
        while (g_send_busy)
        {
            if(millis() - start > 500)
                return 0;
        }
        g_send_busy = TRUE;
        memcpy(g_curr_trans_buf, buffer+sent, tx);
        usb_status status = USB_Class_CDC_Send_Data(g_app_handle, DIC_BULK_IN_ENDPOINT, g_curr_trans_buf, tx);
        if(status != USB_OK)
        {
            g_send_busy = FALSE;
            return sent;
        }
        sent += tx;
    }
    return size;
}
