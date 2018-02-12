/*
  Modem.cpp - Class definitions that provide a generic modem interface.

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
#include "Modem.h"

#include <cstring>
#include <cstdio>

void Modem::init(URCReceiver &receiver) {
    this->receiver = &receiver;
    async_state = MODEM_OK;
    urc_read = 0;
    urc_write = 0; 
}

uint32_t Modem::timeoutCount() {
    return timeout_count;
}

const char* Modem::lastResponse() {
    return respbuffer;
}

void Modem::startSet(const char* cmd) {
    strcpy(cmdbuffer, cmd);
    valoffset = valbuffer;
}

void Modem::appendSet(int value) {
    valoffset += sprintf(valoffset, "%d", value);
}

void Modem::appendSet(const char* value) {
    valoffset += sprintf(valoffset, "%s", value);
}

void Modem::appendSet(char value) {
    *valoffset++ = value;
}

void Modem::appendSet(uint8_t *value, uint32_t len) {
    for(int i=0; i<len; i++)
        *valoffset++ = (char)value[i];
}

modem_result Modem::intermediateSet(char expected, uint32_t timeout, uint32_t retries) {
    checkURC();
    if(async_state == MODEM_BUSY) return MODEM_BUSY;
    do {
        respbuffer[0] = 0;
        modemwrite(cmdbuffer, CMD_STARTAT);
        modemwrite("=");
        modemwrite(valbuffer, CMD_END);
        uint32_t startMillis = msTick();
        while (msTick() - startMillis < timeout) {
            while(modemavailable()) {
                char c = modempeek();
                debugout("<");
                if(c == '\r') {
                    debugout("\\r");
                } else if(c == '\n') {
                    debugout("\\n");
                } else if(c < 0x21 || c > 0x7E)
                    debugout((int)c);
                else {
                    debugout(c);
                }
                debugout(">\r\n");

                if(c == expected) {
                    modemread();
                    return MODEM_OK;
                } else if(c == '+') {
                    if(modemavailable() >= 2)
                        checkURC();
                } else {
                    modemread();
                }
            }
        }
    }while(retries--);
    timeout_count++;
    return MODEM_TIMEOUT;
}

modem_result Modem::waitSetComplete(uint32_t timeout, uint32_t retries)
{
    return waitSetComplete(NULL, timeout, retries);
}

modem_result Modem::waitSetComplete(const char* expected, uint32_t timeout, uint32_t retries) {
    modem_result r = MODEM_TIMEOUT;
    do {
        respbuffer[0] = 0;
        r = processResponse(timeout, cmdbuffer);
        if(r == MODEM_OK) {
            if(expected) {
                if(strcmp(expected, respbuffer) == 0) {
                    return MODEM_OK;
                } else {
                    return MODEM_NO_MATCH;
                }
            } else {
                return MODEM_OK;
            }
        }
    }while(retries--);
    return r;
}

modem_result Modem::completeSet(uint32_t timeout, uint32_t retries) {
    *valoffset = 0;
    set(cmdbuffer, valbuffer, timeout, retries);
}

modem_result Modem::completeSet(const char* expected, uint32_t timeout, uint32_t retries) {
    *valoffset = 0;
    set(cmdbuffer, valbuffer, expected, timeout, retries);
}

bool Modem::findline(char *buffer, uint32_t timeout, uint32_t startMillis) {
    char *rx = buffer;
    while (msTick() - startMillis < timeout) {
        while(modemavailable()) {
            *rx = modemread();
            if(*rx == '\n') {
                while(*rx == '\n' || *rx == '\r') {
                    *rx-- = 0;
                }
                debugout("{");
                debugout(buffer);
                debugout("}\r\n");
                return true;
            } else {
                rx++;
            }
        }
    }
    *rx = 0;
    debugout("{");
    debugout(buffer);
    debugout("}!\r\n");
    return false;
}


void Modem::pushURC(char c)
{
  int slot = nextSlotURC(urc_write);
  if(slot != urc_read)
  {
    urc_buffer[urc_write] = c;
    urc_write = slot;
  }
}

void Modem::pushURC(const char* urc) {
    int len = strlen(urc);
    if(len+1 > remainingURC()) {
        return;
    }
    
    for(int i=0;i<len;i++) {
        pushURC(urc[i]);
    }
    pushURC('\n');
}

int Modem::popURC()
{
    if(urc_read == urc_write) {
        return -1;
    }

    char c = urc_buffer[urc_read];
    urc_read = nextSlotURC(urc_read);

    return c;
}

int Modem::availableURC()
{
    int available = urc_write - urc_read;

    if(available < 0) {
        return URC_BUFFER_SIZE + available;
    }
    else {
        return available;
    }
}

int Modem::remainingURC() {
    return URC_BUFFER_SIZE - availableURC() - 1;
}

int Modem::nextSlotURC(int slot)
{
    return (uint32_t)(slot + 1) % URC_BUFFER_SIZE;
}

bool Modem::findlineURC(char *buffer) {
    char *rx = buffer;
    while(availableURC()) {
        *rx = popURC();
        if(*rx == '\n') {
            while(*rx == '\n' || *rx == '\r') {
                *rx-- = 0;
            }
            return true;
        } else {
            rx++;
        }
    }
    *rx = 0;
    return false;
}

void Modem::modemwrite(const char* cmd, cmd_flags flags) {
    if(flags & CMD_START) {
        debugout("[");
    }
    if(flags & CMD_AT) {
        modemout("AT");
    }
    modemout(cmd);
    if(flags & CMD_QUERY) {
        modemout("?");
    }
    if(flags & CMD_END) {
        debugout("]");
        modemout("\r\n");
    }
}

void Modem::checkURC() {
    if(async_state == MODEM_BUSY) {
        modem_result r = processResponse(10, cmdbuffer);
        if(r == MODEM_TIMEOUT) {
            if(msTick() - async_start >= async_timeout) {
                async_state = MODEM_TIMEOUT;
            }
        } else {
            async_state = r;
        }
        return;
    }

    while(availableURC()) {
        if(findlineURC(okbuffer)) {
            if(receiver) {
                receiver->onURC(okbuffer);
            }
        }
    }

    while(modemavailable()) {
        uint32_t startMillis = msTick();
        if(findline(okbuffer, 100, startMillis)) {
            if(okbuffer[0] == '+') {
                debugout("!URC: '");
                debugout(okbuffer);
                debugout("'\r\n");
                if(receiver) {
                    receiver->onURC(okbuffer);
                }
            }
        }
    }
}

int Modem::strncmpci(const char* str1, const char* str2, size_t num) {
    for(int i=0; i<num; i++) {
        char c1 = str1[i];
        char c2 = str2[i];
        if(c1 >= 'a' && c1 <= 'z') c1 -= 32;
        if(c2 >= 'a' && c2 <= 'z') c2 -= 32;
        if(c1 == c2) {
            if(c1 == 0)
                return 0;
        } else {
            return c1 - c2;
        }
    }
    return 0;
}

bool Modem::commandResponseMatch(const char* cmd, const char* response, int num) {
    if(strncmpci(cmd, response, num) == 0) {
        if(response[num] == ':' && response[num+1] == ' ') {
            return true;
        }
    }
    return false;
}

uint32_t Modem::numResponses() {
    return numresponses;
}

modem_result Modem::processResponse(uint32_t timeout, const char* cmd, int minResponses) {
    uint32_t startMillis = msTick();
    numresponses = 0;
    while(findline(okbuffer, timeout, startMillis)) {
        if(okbuffer[0] == 0) {
            continue;
        } else if(strcmp(okbuffer, "ERROR") == 0) {
            return MODEM_ERROR;
        } else if(strncmp(okbuffer, "+CME ERROR:", 11) == 0) {
            strcpy(respbuffer, okbuffer);
            return MODEM_ERROR;
        } else if(strncmp(okbuffer, "+CMS ERROR:", 11) == 0) {
            strcpy(respbuffer, okbuffer);
            return MODEM_ERROR;
        } else if(strcmp(okbuffer, "OK") == 0 && numresponses >= minResponses) {
            timeout_count = 0;
            return MODEM_OK;
        } else if(okbuffer[0] == '+') {
            timeout_count = 0;
            if(commandResponseMatch(cmd, okbuffer, strlen(cmd))) {
                numresponses++;
                strcpy(respbuffer, okbuffer);
            } else {
                debugout(">URC: '");
                debugout(okbuffer);
                debugout("'\r\n");
                pushURC(okbuffer);
            }
            startMillis = msTick();
        } else if(strncmp(okbuffer, "AT", 2) == 0 && strncmp(&okbuffer[2], cmd, strlen(cmd)) == 0) {
            debugout(">ECHO: '");
            debugout(okbuffer);
            debugout("'\r\n");
        } else {
            strcpy(respbuffer, okbuffer);
        }
    }
    timeout_count++;
    return MODEM_TIMEOUT;
}

modem_result Modem::command(const char* cmd, const char* expected, uint32_t timeout, uint32_t retries, bool query) {
    checkURC();
    if(async_state == MODEM_BUSY) return MODEM_BUSY;
    modem_result r = MODEM_TIMEOUT;
    do {
        respbuffer[0] = 0;
        modemwrite(cmd, query ? CMD_FULL_QUERY : CMD_FULL);
        r = processResponse(timeout, cmd);
        if(r == MODEM_OK) {
            if(expected) {
                if(strcmp(expected, respbuffer) == 0) {
                    return MODEM_OK;
                } else {
                    return MODEM_NO_MATCH;
                }
            } else {
                return MODEM_OK;
            }
        }
    }while(retries--);
    return r;
}

modem_result Modem::command(const char* cmd, uint32_t timeout, uint32_t retries, bool query) {
    return command(cmd, NULL, timeout, retries, query);
}

modem_result Modem::asyncStatus() {
    return async_state;
}

modem_result Modem::asyncSet(const char* cmd, const char* value, uint32_t timeout) {
    checkURC();
    if(async_state == MODEM_BUSY) return MODEM_BUSY;
    respbuffer[0] = 0;
    strcpy(cmdbuffer, cmd);
    modemwrite(cmd, CMD_STARTAT);
    modemwrite("=");
    modemwrite(value, CMD_END);
    async_state = MODEM_BUSY;
    async_timeout = timeout;
    async_start = msTick();
    return MODEM_OK;
}

modem_result Modem::set(const char* cmd, const char* value, const char* expected, uint32_t timeout, uint32_t retries) {
    checkURC();
    if(async_state == MODEM_BUSY) return MODEM_BUSY;
    modem_result r = MODEM_TIMEOUT;
    do {
        respbuffer[0] = 0;
        modemwrite(cmd, CMD_STARTAT);
        modemwrite("=");
        modemwrite(value, CMD_END);
        r = processResponse(timeout, cmd);
        if(r == MODEM_OK) {
            if(expected) {
                if(strcmp(expected, respbuffer) == 0) {
                    debugout(">set match: '");
                    debugout(expected);
                    debugout("' '");
                    debugout(respbuffer);
                    debugout("'\r\n");
                    return MODEM_OK;
                } else {
                    return MODEM_NO_MATCH;
                }
            } else {
                return MODEM_OK;
            }
        }
    }while(retries--);
    return r;
}

modem_result Modem::set(const char* cmd, const char* value, uint32_t timeout, uint32_t retries) {
    return set(cmd, value, NULL, timeout, retries);
}

void Modem::rawWrite(char c) {
    modemout(c);
}

void Modem::rawWrite(const char* content) {
    modemwrite(content);
}

void Modem::dataWrite(uint8_t b) {
    modemout(b);
}

void Modem::dataWrite(const uint8_t* content, uint32_t length) {
    for(uint32_t i=0; i<length; i++)
        modemout(content[i]);
}

void Modem::rawRead(int length, void* buffer) {
    uint32_t startMillis = msTick();
    uint32_t timeout = 30000;
    uint8_t* pbuffer = (uint8_t*)buffer;
    int read = 0;
    while(read < length && msTick() - startMillis < timeout) {
        if(modemavailable()) {
            if(buffer) {
                pbuffer[read++] = modemread();
            } else {
                modemread();
            }
        }
    }
    if(buffer) {
        pbuffer[read] = 0;
    }
}

uint8_t Modem::convertHex(char hex) {
    uint8_t x = 0;
    if(hex >= '0' && hex <= '9') {
        x = hex - '0';
    } else if(hex >= 'A' && hex <= 'F') {
        x = hex - 'A' + 10;
    } else if(hex >= 'a' && hex <= 'f') {
        x = hex - 'a' + 10;
    }
    return x;
}

uint8_t Modem::convertHex(const char* hex) {
    uint8_t x = convertHex(hex[0]) << 4;
    x |= convertHex(hex[1]);
    return x;
}
