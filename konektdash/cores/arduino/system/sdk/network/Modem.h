#pragma once

#include <cstdint>
#include <cstddef> 

typedef enum {
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
    const char* lastResponse();
    void checkURC();
    void rawWrite(char c);
    void rawWrite(const char* content);
    void dataWrite(const uint8_t* content, uint32_t length);
    void dataWrite(uint8_t b);
    void rawRead(int length, void* buffer);
    virtual uint32_t msTick()=0;

protected:
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
    virtual bool modemavailable()=0;
    virtual uint8_t modemread()=0;
    virtual uint8_t modempeek()=0;
    void modemwrite(const char* cmd, cmd_flags flags = CMD_NONE);
    bool findline(char *buffer, uint32_t timeout, uint32_t startMillis);
    modem_result processResponse(uint32_t timeout, const char* cmd);
    int strncmpci(const char* str1, const char* str2, size_t num);

    URCReceiver *receiver;
    char cmdbuffer[32];
    char valbuffer[48];
    char respbuffer[64];
    char okbuffer[64];
    char *valoffset;
};
