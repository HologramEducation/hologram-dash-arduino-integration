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

#define CLOUD_DISCONNECTED      0
#define CLOUD_CONNECTED         1
#define CLOUD_ERR_UNAVAILABLE   2
#define CLOUD_ERR_SIM           3
#define CLOUD_ERR_UNREGISTERED  4
#define CLOUD_ERR_SIGNAL        5
#define CLOUD_ERR_CONNECT       12

typedef enum {
    CLOUD_EVENT_NONE            = 0,
    CLOUD_EVENT_DISCONNECTED    = 1,
    CLOUD_EVENT_UNREGISTERED    = 2,
    CLOUD_EVENT_REGISTERED      = 3,
}cloud_event;

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

    String getICCID();
    String getNetworkOperator();
    bool getLocation(int accuracy=10, int maxseconds=360);

    bool enterPassthrough();

    bool sendMessage(const char* content);
    bool sendMessage(const char* content, const char* topic);
    bool sendMessage(const uint8_t* content, uint32_t length);
    bool sendMessage(const uint8_t* content, uint32_t length, const char* topic);
    bool sendMessage(const String &content);
    bool sendMessage(const String &content, const String &topic);
    bool sendMessage(const String &content, const char* topic);
    bool sendMessage(const char* content, const String &topic);
    bool sendMessage(const uint8_t* content, uint32_t length, const String &topic);

    //deprecated
    #define attachTag attachTopic

    bool attachTopic(const char* topic);
    bool attachTopic(const String &topic) {return attachTopic(topic.c_str());}
    size_t write(uint8_t x);
    bool sendMessage();

    int listen(int port);

    void attachHandlerSMS(void (*sms_handler)(const String &sender, const rtc_datetime_t &timestamp, const String &message));
    void attachHandlerInbound(void (*inbound_handler)(int length), void *buffer, int length);
    void attachHandlerNotify(void (*event_handler)(cloud_event e));
    void attachHandlerLocation(void (*location_handler)(const rtc_datetime_t &timestamp, const String &lat, const String &lon, int altitude, int uncertainty));
    void onURC(const char* urc);

protected:
    typedef enum {
        MODEM_STATE_UNKNOWN,
        MODEM_STATE_READY,
        MODEM_STATE_OFF,
    }state_modem;

    void resetSystem();
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
    uint8_t *inbound_buffer;
    int inbound_length;
    uint8_t message_buffer[MAX_MESSAGE_SIZE];
    uint32_t message_length;
    char topics[MAX_TOPICS][MAX_TOPIC_SIZE+1];
    uint32_t num_topics;
    bool ready;
    bool auto_reconnect;
    state_modem modem_state;
    bool message_attempted;
    int32_t protocol_version;
    int inbound_pending;
};
