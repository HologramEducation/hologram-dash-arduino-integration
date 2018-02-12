/*
  Modem.h - Class definitions that provide a generic modem interface.

  https://hologram.io

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

#include <cstdint>
#include <cstddef>

typedef enum {
    MODEM_BUSY = -4,
    MODEM_NO_MATCH = -3,
    MODEM_ERROR = -2,
    MODEM_TIMEOUT = -1,
    MODEM_OK = 0,
}modem_result;

class URCReceiver {
public:
    virtual void onURC(const char* urc)=0;
};

class Modem {
public:
    void init(URCReceiver &receiver);
    modem_result command(const char* cmd, uint32_t timeout=1000, uint32_t retries=0, bool query=false);
    modem_result command(const char* cmd, const char* expected, uint32_t timeout=1000, uint32_t retries=0, bool query=false);
    modem_result set(const char* cmd, const char* value, uint32_t timeout=1000, uint32_t retries=0);
    modem_result set(const char* cmd, const char* value, const char* expected, uint32_t timeout=1000, uint32_t retries=0);
    modem_result asyncSet(const char* cmd, const char* value, uint32_t timeout=1000);
    modem_result asyncStatus();
    void startSet(const char* cmd);
    void appendSet(int value);
    void appendSet(const char* value);
    void appendSet(char value);
    void appendSet(uint8_t *value, uint32_t len);
    modem_result completeSet(uint32_t timeout=1000, uint32_t retries=0);
    modem_result completeSet(const char* expected, uint32_t timeout=1000, uint32_t retries=0);
    modem_result intermediateSet(char expected, uint32_t timeout=1000, uint32_t retries=0);
    modem_result waitSetComplete(uint32_t timeout=1000, uint32_t retries=0);
    modem_result waitSetComplete(const char* expected, uint32_t timeout=1000, uint32_t retries=0);
    modem_result query(const char* cmd, uint32_t timeout=1000, uint32_t retries=0) {
        return command(cmd, timeout, retries, true);
    }
    modem_result query(const char* cmd, const char* expected, uint32_t timeout=1000, uint32_t retries=0) {
        return command(cmd, expected, timeout, retries, true);
    }
    uint32_t timeoutCount();
    const char* lastResponse();
    uint32_t numResponses();
    void checkURC();
    void rawWrite(char c);
    void rawWrite(const char* content);
    void dataWrite(const uint8_t* content, uint32_t length);
    void dataWrite(uint8_t b);
    void rawRead(int length, void* buffer);
    virtual uint32_t msTick()=0;

    static uint8_t convertHex(char hex);
    static uint8_t convertHex(const char* hex);

protected:
    #define URC_BUFFER_SIZE 256
    typedef enum {
        CMD_NONE  = 0x00,
        CMD_START = 0x01,
        CMD_AT    = 0x02,
        CMD_QUERY = 0x04,
        CMD_END   = 0x08,

        CMD_STARTAT = 0x03,
        CMD_FULL  = 0x0B,
        CMD_FULL_QUERY = 0x0F,
    }cmd_flags;

    virtual void modemout(char c)=0;
    virtual void modemout(const char* str)=0;
    virtual void modemout(uint8_t b)=0;
    virtual void debugout(const char* str){}
    virtual void debugout(char c){}
    virtual void debugout(int i){}
    virtual int modemavailable()=0;
    virtual uint8_t modemread()=0;
    virtual uint8_t modempeek()=0;
    void modemwrite(const char* cmd, cmd_flags flags = CMD_NONE);
    bool findline(char *buffer, uint32_t timeout, uint32_t startMillis);
    modem_result processResponse(uint32_t timeout, const char* cmd, int minResponse=0);
    int strncmpci(const char* str1, const char* str2, size_t num);
    bool commandResponseMatch(const char* cmd, const char* response, int num);

    void pushURC(char c);
    void pushURC(const char* urc);
    int popURC();
    int availableURC();
    int remainingURC();
    int nextSlotURC(int slot);
    bool findlineURC(char *buffer);

    URCReceiver *receiver;
    char cmdbuffer[32];
    char valbuffer[48];
    char respbuffer[512];
    char okbuffer[512];
    char *valoffset;
    uint32_t numresponses;
    modem_result async_state;
    uint32_t async_start;
    uint32_t async_timeout;
    uint32_t timeout_count;
    char urc_buffer[URC_BUFFER_SIZE];
    int urc_write;
    int urc_read;
};
