/*
  Hologram.cpp - Provides IPC layer to system processor and Hologram Cloud

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

#include "Hologram.h"
#include "Wire.h"
#include "wiring_digital.h"
#include "wiring_constants.h"
#include "WInterrupts.h"
#include "Dash.h"

const uint8_t MODEM_STATE_UNKNOWN = 0;
const uint8_t MODEM_STATE_READY = 0;
const uint8_t MODEM_STATE_OFF = 0;

void Hologram::attachHandlerSMS(void (*sms_handler)(const String &sender, const rtc_datetime_t &timestamp, const String &message)) {
    sms_callback = sms_handler;
    if(sms_callback) {
        while(checkSMS() > 0) {
            modem.command("+HSMSRD");
            delay(100);
            pollEvents();
        }
    }
}

void Hologram::begin() {
    modem.begin(SerialSystem, *this);
    ready = true;
    powerUp();
}

void Hologram::end() {
    ready = false;
}

int Hologram::checkSMS() {
    if(modem.query("+HSMS") == MODEM_OK) {
        int num_sms;
        if(sscanf(modem.lastResponse(), "+HSMS: %d", &num_sms) == 1) {
            return num_sms;
        }
    }
    return 0;
}

void Hologram::onURC(const char* urc) {
    if(strncmp(urc, "+HSMSRX: %d", 9) == 0) {
        //SMS Available, but do polling instead
    } else if(strncmp(urc, "+HSMSCTX: ", 10) == 0) {
        int msglen = 0;
        rtc_datetime_t dt;
        int numparams = sscanf(urc, "+HSMSCTX: \"%[^\"]\",\"%d/%d/%d,%d:%d:%d\",%d", sms_sender, &dt.year, &dt.month, &dt.day, &dt.hour, &dt.minute, &dt.second, &msglen);
        if(numparams == 8) {
            modem.rawRead(msglen, sms_message);
            if(sms_callback) {
                sms_callback(String(sms_sender), dt, String(sms_message));
            }
        }
    } else if(strncmp(urc, "+HDISCONNECTED: ", 16) == 0) {
        if(auto_reconnect) {
            connect(true);
        }
    } else if(strncmp(urc, "+HINFO: ", 8) == 0) {
        modem_state = MODEM_STATE_READY;
    }
 }

void Hologram::resetSystem() {
    pinMode(26, OUTPUT);
    digitalWrite(26, LOW);
    delay(50);
    pinMode(26, INPUT);
    delay(50);
}

//blocking...
bool Hologram::connect(bool reconnect) {
    powerUp();
    auto_reconnect = reconnect;
    if(modem.command("+HCONNECT", 3*60*1000) == MODEM_OK) {
        int status = 2;
        if(sscanf(modem.lastResponse(), "+HCONNECT: %d", &status) == 1) {
            return status == CLOUD_CONNECTED;
        }
    }
    return false;
}

bool Hologram::disconnect() {
    powerUp();
    auto_reconnect = false;
    return modem.command("+HDISCONNECT", 30*1000) == MODEM_OK;
}

int Hologram::getConnectionStatus() {
    int status = 2;
    powerUp();
    if(modem.command("+HCONSTATUS") == MODEM_OK) {
        sscanf(modem.lastResponse(), "+HCONSTATUS: %d", &status);
    }
    return status;
}

int Hologram::getSignalStrength() {
    powerUp();
    if(modem.command("+CSQ") != MODEM_OK)
        return 99;
    int rssi = 99;
    int qual = 0;
    sscanf(modem.lastResponse(), "+CSQ: %d,%d", &rssi, &qual);
    return rssi;
}

bool Hologram::getNetworkTime(rtc_datetime_t &dt) {
    if(modem.query("+CCLK") != MODEM_OK) {
        return false;
    }
    if(sscanf(modem.lastResponse(), "+CCLK: \"%d/%d/%d,%d:%d:%d", &dt.year, &dt.month, &dt.day, &dt.hour, &dt.minute, &dt.second) == 6) {
        if(dt.year == 4) {
            if(getSignalStrength() == 99)
                return false;
        }
        dt.year += 2000;
        return true;
    }
    return false;
}

String Hologram::systemVersion() {
    char ver[9];
    int value=0;
    if(modem.set("+HSYS", "2") == MODEM_OK) {
        if(sscanf(modem.lastResponse(), "+HSYS: %d,\"%[^\"]\"", &value, ver) == 2) {
            if(value == 2)
                return String(ver);
        }
    }
    return String("0.0.0");
}

void Hologram::powerUp() {
    int retries = 30;
    if(modem_state == MODEM_STATE_READY) {
        retries = 5;
    } else if(modem_state == MODEM_STATE_OFF) {
        resetSystem();
        Dash.snooze(1000);
    }
    while(modem.command("", 100, retries) != MODEM_OK) {
        resetSystem();
        Dash.snooze(1000);
        retries *= 2;
    }
    modem_state == MODEM_STATE_READY;
}

void Hologram::powerDown() {
    pollEvents();
    modem_state = MODEM_STATE_OFF;
    modem.command("+HSHUTDOWN");
}

void Hologram::pollEvents() {
    //check SMS pending...
    if(ready && modem_state == MODEM_STATE_READY) {
        modem.checkURC();

        while(checkSMS() > 0) {
            modem.command("+HSMSRD");
            delay(100);
            modem.checkURC();
        }
    }
}

size_t Hologram::write(uint8_t x) {
    resetBuffer();
    if(message_length < MAX_MESSAGE_SIZE) {
        message_buffer[message_length++] = x;
        return 1;
    }
    return 0;
}

bool Hologram::attachTag(const char* tag) {
    if(strlen(tag) > MAX_TAG_SIZE) return false;
    resetBuffer();
    if(num_tags == MAX_TAGS) return false;
    strcpy(tags[num_tags++], tag);
    return true;
}

void Hologram::clear() {
    num_tags = 0;
    message_length = 0;
}

bool Hologram::sendFinalize(bool success) {
    message_attempted = true;
    return success;
}

void Hologram::resetBuffer() {
    if(message_attempted) {
        clear();
        message_attempted = false;
    }
}

bool Hologram::sendMessage() {
    message_attempted = false;

    if(!connect(auto_reconnect))
        return sendFinalize(false);

    if(modem.command("+HMRST") != MODEM_OK)
        return sendFinalize(false);

    for(int i=0; i<num_tags; i++) {
        modem.set("+HTAG", tags[i]);
    }

    uint32_t length = message_length;

    uint32_t wrcount = 0;
    while(length > 0) {
        int tosend = length;
        if(tosend > 128) tosend = 128;
        modem.startSet("+HMWRITE");
        modem.appendSet(tosend);
        if(modem.intermediateSet('@', 10000) == MODEM_OK) {
            for(int i=0; i<tosend; i++)
                modem.dataWrite(message_buffer[wrcount++]);
        }
        if(modem.waitSetComplete(10000) == MODEM_OK) {
            length -= tosend;
        } else {
            return sendFinalize(false);
        }
    }

    return sendFinalize(modem.command("+HMSEND", 3*60*1000) == MODEM_OK);
}

bool Hologram::sendMessage(const String &content) {
    return sendMessage(content.c_str());
}

bool Hologram::sendMessage(const char* content) {
    return sendMessage((const uint8_t*)content, strlen(content));
}

bool Hologram::sendMessage(const String &content, const String &tag) {
    return sendMessage(content.c_str(), tag.c_str());
}

bool Hologram::sendMessage(const char* content, const char* tag) {
    return sendMessage((const uint8_t*)content, strlen(content), tag);
}

bool Hologram::sendMessage(const String &content, const char* tag) {
    return sendMessage(content.c_str(), tag);
}

bool Hologram::sendMessage(const char* content, const String &tag) {
    return sendMessage(content, tag.c_str());
}

bool Hologram::sendMessage(const uint8_t* content, uint32_t length) {
    resetBuffer();

    if(length+message_length > MAX_MESSAGE_SIZE) {
        length = MAX_MESSAGE_SIZE - message_length;
    }
    memcpy(&message_buffer[message_length], content, length);
    message_length += length;

    return sendMessage();
}

bool Hologram::sendMessage(const uint8_t* content, uint32_t length, const String &tag) {
    return sendMessage(content, length, tag.c_str());
}

bool Hologram::sendMessage(const uint8_t* content, uint32_t length, const char* tag) {
    if(tag)
        attachTag(tag);
    return sendMessage(content, length);
}
