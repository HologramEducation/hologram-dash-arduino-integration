/*
  Hologram.h - Provides IPC layer to system processor and Hologram Cloud

  http://hologram.io

  Copyright (c) 2017 Konekt, Inc.  All rights reserved.

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
#pragma once

#include "Print.h"
#include "system/hal/ArduinoModem.h"
#include "hal/fsl_rtc_hal.h"

#define MAX_MESSAGE_SIZE 4096
#define MAX_TAG_SIZE 63
#define MAX_TAGS 10

#define CLOUD_DISCONNECTED  0
#define CLOUD_CONNECTED     1
#define CLOUD_ERR_SIM       3
#define CLOUD_ERR_SIGNAL    5
#define CLOUD_ERR_CONNECT   12

class Hologram : public Print, public URCReceiver {
public:
    void begin();
    void end();

    bool connect(bool reconnect=false);
    bool disconnect();
    int getConnectionStatus();
    int getSignalStrength();
    bool getNetworkTime(rtc_datetime_t &dt);

    void powerUp();
    void powerDown();

    void pollEvents();

    void clear();

    String systemVersion();
    int checkSMS();

    bool sendMessage(const char* content);
    bool sendMessage(const char* content, const char* tag);
    bool sendMessage(const uint8_t* content, uint32_t length);
    bool sendMessage(const uint8_t* content, uint32_t length, const char* tag);
    bool sendMessage(const String &content);
    bool sendMessage(const String &content, const String &tag);
    bool sendMessage(const String &content, const char* tag);
    bool sendMessage(const char* content, const String &tag);
    bool sendMessage(const uint8_t* content, uint32_t length, const String &tag);

    bool attachTag(const char* tag);
    bool attachTag(const String &tag) {return attachTag(tag.c_str());}
    size_t write(uint8_t x);
    bool sendMessage();

    void attachHandlerSMS(void (*sms_handler)(const String &sender, const rtc_datetime_t &timestamp, const String &message));

    void onURC(const char* urc);

protected:
    void resetSystem();
    bool sendFinalize(bool success);
    void resetBuffer();

    char sms_sender[21];
    char sms_message[161];
    void (*sms_callback)(const String &sender, const rtc_datetime_t &timestamp, const String &message);
    uint8_t message_buffer[MAX_MESSAGE_SIZE];
    uint32_t message_length;
    char tags[MAX_TAGS][MAX_TAG_SIZE+1];
    uint32_t num_tags;
    bool ready;
    bool auto_reconnect;
    uint8_t modem_state;
    bool message_attempted;
};
