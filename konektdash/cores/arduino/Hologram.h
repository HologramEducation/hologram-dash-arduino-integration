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
#define MAX_TOPIC_SIZE 63
#define MAX_TOPICS 10

#define CLOUD_REGISTERED        0
#define CLOUD_CONNECTED         1
#define CLOUD_ERR_UNAVAILABLE   2
#define CLOUD_ERR_SIM           3
#define CLOUD_ERR_UNREGISTERED  4
#define CLOUD_ERR_SIGNAL        5
#define CLOUD_ERR_CONNECT       12
#define CLOUD_ERR_MODEM_OFF     15

typedef enum {
    CLOUD_EVENT_NONE            = 0,
    CLOUD_EVENT_DISCONNECTED    = 1,
    CLOUD_EVENT_UNREGISTERED    = 2,
    CLOUD_EVENT_REGISTERED      = 3,
    CLOUD_EVENT_CONNECTED       = 4,
    CLOUD_EVENT_RESET           = 99,
}cloud_event;

typedef enum {
    CHARGE_STATUS_FAULT         = 0,
    CHARGE_STATUS_INVALID1      = 1,
    CHARGE_STATUS_CHARGING      = 2,
    CHARGE_STATUS_LOW_BATTERY   = 3,
    CHARGE_STATUS_CHARGED       = 4,
    CHARGE_STATUS_INVALID5      = 5,
    CHARGE_STATUS_NO_BATTERY    = 6,
    CHARGE_STATUS_NO_INPUT      = 7,
}charge_status;

class Hologram : public Print, public URCReceiver {
public:
    void begin();
    void end();

    bool connect();
    bool disconnect();
    int getConnectionStatus();
    bool isConnected();
    bool isRegistered();
    int getSignalStrength();
    bool getNetworkTime(rtc_datetime_t &dt);
    bool getUTC(rtc_datetime_t &dt);

    void powerUp();
    void powerDown();

    void pollEvents();

    void clear();

    String systemVersion();
    int checkSMS();

    String getICCID();
    String getIMEI();
    String getNetworkOperator();
    bool getLocation(int accuracy=10, int maxseconds=360);

    int getChargeState();

    bool enterPassthrough();

    bool setRGB(String name);
    bool setRGB(int hexcode);
    bool offRGB() {return setRGB("BLACK");}
    bool setLED(bool on);

    bool sendMessage(const char* content);
    bool sendMessage(const char* content, const char* topic);
    bool sendMessage(const uint8_t* content, uint32_t length);
    bool sendMessage(const uint8_t* content, uint32_t length, const char* topic);
    bool sendMessage(const String &content);
    bool sendMessage(const String &content, const String &topic);
    bool sendMessage(const String &content, const char* topic);
    bool sendMessage(const char* content, const String &topic);
    bool sendMessage(const uint8_t* content, uint32_t length, const String &topic);

    int sendTimeout();
    int sendDelay(bool ok, int milliseconds);

    //deprecated
    #define attachTag attachTopic

    bool attachTopic(const char* topic);
    bool attachTopic(const String &topic) {return attachTopic(topic.c_str());}
    size_t write(uint8_t x);
    bool sendMessage();

    int listen(int port);

    void resetSystem();

    void attachHandlerSMS(void (*sms_handler)(const String &sender, const rtc_datetime_t &timestamp, const String &message));
    void attachHandlerInbound(void (*inbound_handler)(int length), void *buffer, int length);
    void attachHandlerNotify(void (*event_handler)(cloud_event e));
    void attachHandlerLocation(void (*location_handler)(const rtc_datetime_t &timestamp, const String &lat, const String &lon, int altitude, int uncertainty));
    void attachHandlerCharge(void (*charge_handler)(charge_status status));
    void onURC(const char* urc);

protected:
    typedef enum {
        MODEM_STATE_UNKNOWN,
        MODEM_STATE_SHUTDOWN,
        MODEM_STATE_DISCONNECTED,
        MODEM_STATE_READY,
    }state_modem;

    bool getTime(rtc_datetime_t &dt, bool utc);

    bool sendFinalize(bool success);
    void resetBuffer();
    void checkIncoming();
    void notifySMS();
    int read(int socket, void *buffer, int max_len, int timeout=10000);
    void close(int socket);

    char sms_sender[21];
    char sms_message[161];
    rtc_datetime_t sms_dt;
    rtc_datetime_t loc_dt;
    char loc_lat[16];
    char loc_lon[16];
    bool sms_pending;
    void (*sms_callback)(const String &sender, const rtc_datetime_t &timestamp, const String &message);
    void (*inbound_callback)(int length);
    void (*event_callback)(cloud_event e);
    void (*location_callback)(const rtc_datetime_t &timestamp, const String &lat, const String &lon, int altitude, int uncertainty);
    void (*charge_callback)(charge_status status);
    uint8_t *inbound_buffer;
    int inbound_length;
    uint8_t message_buffer[MAX_MESSAGE_SIZE];
    uint32_t message_length;
    char topics[MAX_TOPICS][MAX_TOPIC_SIZE+1];
    uint32_t num_topics;
    bool ready;
    state_modem modem_state;
    bool message_attempted;
    int32_t protocol_version;
    int inbound_pending;
};

extern ArduinoModem modem;
