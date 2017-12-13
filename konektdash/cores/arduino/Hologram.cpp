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

void Hologram::attachHandlerSMS(void (*sms_handler)(const String &sender, const rtc_datetime_t &timestamp, const String &message)) {
    sms_callback = sms_handler;
    pollEvents();
}

void Hologram::attachHandlerInbound(void (*inbound_handler)(int length), void *buffer, int length) {
    inbound_callback = inbound_handler;
    inbound_buffer = (uint8_t*)buffer;
    inbound_length = length;
}

void Hologram::attachHandlerNotify(void (*event_handler)(cloud_event e)) {
    event_callback = event_handler;
}

void Hologram::attachHandlerLocation(void (*location_handler)(const rtc_datetime_t &timestamp, const String &lat, const String &lon, int altitude, int uncertainty)) {
    location_callback = location_handler;
}

void Hologram::attachHandlerCharge(void (*charge_handler)(charge_status status)) {
    charge_callback = charge_handler;
}

void Hologram::begin() {
    end();
    // Serial2.begin(115200);
    // modem.begin(SerialSystem, *this, &Serial2);
    modem.begin(SerialSystem, *this);
    ready = true;
    powerUp();
}

void Hologram::end() {
    ready = false;
    protocol_version = 0;
    modem_state = MODEM_STATE_UNKNOWN;
    message_attempted = false;
    num_topics = 0;
    inbound_pending = 0;
    sms_pending = false;
}

bool Hologram::enterPassthrough() {
    powerUp();
    if(modem.set("+HPASSTHROUGH", "1") == MODEM_OK) {
        end();
        return true;
    }
    return false;
}

int Hologram::checkSMS() {
    if(modem_state != MODEM_STATE_READY) return 0;
    if(modem.query("+HSMS") == MODEM_OK) {
        int num_sms;
        if(sscanf(modem.lastResponse(), "+HSMS: %d", &num_sms) == 1) {
            return num_sms;
        }
    }
    return 0;
}

void Hologram::checkIncoming() {
    if(inbound_pending == 0 || modem_state != MODEM_STATE_READY) return;
    int readnum = 0;
    int total = 0;
    while(readnum != -1 && total < inbound_length) {
      readnum = HologramCloud.read(inbound_pending, &inbound_buffer[total], inbound_length-total);
      if(readnum > 0)
        total += readnum;
    }
    if(readnum != -1)
        close(inbound_pending);
    inbound_pending = 0;
    inbound_callback(total);
}

void Hologram::onURC(const char* urc) {
    if(strncmp(urc, "+HHSMSRX: %d", 9) == 0) {
        //SMS Available, but do polling instead
    } else if(strncmp(urc, "+HHSMSCTX: ", 10) == 0) {
        int msglen = 0;
        int numparams = sscanf(urc, "+HHSMSCTX: \"%[^\"]\",\"%d/%d/%d,%d:%d:%d\",%d", sms_sender, &sms_dt.year, &sms_dt.month, &sms_dt.day, &sms_dt.hour, &sms_dt.minute, &sms_dt.second, &msglen);
        if(numparams == 8) {
            modem.rawRead(msglen, sms_message);
            sms_pending = true;
        }
    } else if(strncmp(urc, "+HHCONNECTED: ", 14) == 0) {
        int con = 0;
        if(event_callback) {
            if(sscanf(urc, "+HHCONNECTED: %d", &con) == 1) {
                event_callback(con == 1 ? CLOUD_EVENT_CONNECTED : CLOUD_EVENT_DISCONNECTED);
            }
        }
    } else if(strncmp(urc, "+HHREGISTERED: ", 15) == 0) {
        int status = 99;
        if(sscanf(urc, "+HHREGISTERED: %d", &status) == 1) {
            if(event_callback) {
                if(status == 0)
                    event_callback(CLOUD_EVENT_UNREGISTERED);
                else if(status == 1)
                    event_callback(CLOUD_EVENT_REGISTERED);
            }
        }
    } else if(strncmp(urc, "+HHOLO: ", 8) == 0) {
        sscanf(urc, "+HHOLO: %d", &protocol_version);
        modem_state = MODEM_STATE_READY;
        if(event_callback) {
            event_callback(CLOUD_EVENT_RESET);
        }
    } else if(strncmp(urc, "+HHSOCKACCEPT: ", 14) == 0) {
        int id, port, listener;
        char host[16];
        if(sscanf(urc, "+HHSOCKACCEPT: %d,\"%[^\"]\",%d,%d", &id, host, &port, &listener) == 4) {
            if(inbound_callback && inbound_buffer && inbound_length > 0 && inbound_pending == 0) {
                inbound_pending = id;
            } else {
                close(id);
            }
        }
    } else if(strncmp(urc, "+HHLOC: ", 8) == 0) {
        //+HHLOC: "2011/04/13,09:54:51",45.6334520,13.0618620,49,1
        int altitude, uncertainty;
        int numparams = sscanf(urc, "+HHLOC: \"%d/%d/%d,%d:%d:%d\",%[^,],%[^,],%d,%d", &loc_dt.year, &loc_dt.month, &loc_dt.day, &loc_dt.hour, &loc_dt.minute, &loc_dt.second, loc_lat, loc_lon, &altitude, &uncertainty);
        if(numparams == 10) {
            if(location_callback) {
                location_callback(loc_dt, loc_lat, loc_lon, altitude, uncertainty);
            }
        }
    } else if(strncmp(urc, "+HHCHARGE: ", 11) == 0) {
        int charge_state = 0;
        if(sscanf(urc, "+HHCHARGE: %d", &charge_state) == 1) {
            if(charge_callback) {
                charge_callback((charge_status)(charge_state));
            }
        }
    }
}

void Hologram::resetSystem() {
    pinMode(26, OUTPUT);
    digitalWrite(26, LOW);
    Dash.snooze(10);
    pinMode(26, INPUT);
    Dash.snooze(50);
    pinMode(26, DISABLE);
}

int Hologram::listen(int port) {
    if(modem_state == MODEM_STATE_READY) {
        modem.startSet("+HSOCKLISTEN");
        modem.appendSet(port);
        if(modem.completeSet(10*1000) == MODEM_OK) {
            int socket;
            if(sscanf(modem.lastResponse(), "+HSOCKLISTEN: %d", &socket) == 1) {
                return socket;
            }
        }
    }
    return 0;
}

int Hologram::read(int socket, void *buffer, int max_len, int timeout) {
    modem.checkURC();

    modem.startSet("+HSOCKREAD");
    modem.appendSet(socket);
    modem.appendSet(",");
    modem.appendSet(max_len);
    modem.appendSet(",");
    modem.appendSet(timeout);
    modem.appendSet(",1"); //hex mode
    if(modem.completeSet(timeout+1000) == MODEM_OK) {
        int socket, hex, actual_read;
        if(sscanf(modem.lastResponse(), "+HSOCKREAD: %d,%d,%d,", &socket, &hex, &actual_read) == 3) {
            if(actual_read > 0) {
                const char* q = strchr(modem.lastResponse(), '"');
                if(hex == 1) {
                    for(int i=0; i<actual_read; i++) {
                        ((uint8_t*)buffer)[i] = Modem::convertHex(&q[1+i*2]);
                    }
                } else {
                    memcpy(buffer, &q[1], actual_read);
                }
            }
            return actual_read;
        }
    }
    return -1;
}

void Hologram::close(int socket) {
    if(modem_state == MODEM_STATE_READY) {
        modem.startSet("+HSOCKCLOSE");
        modem.appendSet(socket);
        modem.completeSet(3*1000);
    }
}

bool Hologram::connect() {
    powerUp();
    return true;
}

bool Hologram::disconnect() {
    if(modem_state == MODEM_STATE_READY) {
        if(modem.command("+HDISCONNECT", 30*1000) == MODEM_OK) {
            modem_state = MODEM_STATE_DISCONNECTED;
        }
    }
    return true;
}

bool Hologram::isConnected() {
    return getConnectionStatus() == CLOUD_CONNECTED;
}

bool Hologram::isRegistered() {
    int s = getConnectionStatus();
    return (s == CLOUD_CONNECTED) || (s == CLOUD_REGISTERED);
}

int Hologram::getConnectionStatus() {
    int status = CLOUD_ERR_UNAVAILABLE;
    if(modem_state > MODEM_STATE_SHUTDOWN) {
        if(modem.command("+HCONSTATUS") == MODEM_OK) {
            sscanf(modem.lastResponse(), "+HCONSTATUS: %d", &status);
        }
        return status;
    }
    return CLOUD_ERR_MODEM_OFF;
}

int Hologram::sendTimeout() {
    return modem.command("+HDEBUGTIMEOUT");
}

int Hologram::sendDelay(bool ok, int milliseconds) {
    modem.startSet(ok ? "+HDEBUGDELAY" : "+HDEBUGDELAYERR");
    modem.appendSet(milliseconds);
    return modem.completeSet();
}

int Hologram::getSignalStrength() {
    if(modem_state == MODEM_STATE_DISCONNECTED) return 99;
    powerUp();
    if(modem.command("+CSQ") != MODEM_OK)
        return 99;
    int rssi = 99;
    int qual = 0;
    sscanf(modem.lastResponse(), "+CSQ: %d,%d", &rssi, &qual);
    return rssi;
}

bool Hologram::getNetworkTime(rtc_datetime_t &dt) {
    if(modem_state == MODEM_STATE_DISCONNECTED) return false;
    powerUp();
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

bool Hologram::getUTC(rtc_datetime_t &dt) {
    if(modem_state == MODEM_STATE_DISCONNECTED) return false;
    powerUp();
    if(modem.query("+CCLK") != MODEM_OK) {
        return false;
    }

    int tz;
    if(sscanf(modem.lastResponse(), "+CCLK: \"%d/%d/%d,%d:%d:%d%d", &dt.year, &dt.month, &dt.day, &dt.hour, &dt.minute, &dt.second, &tz) == 7) {
        if(dt.year == 4) {
            if(getSignalStrength() == 99)
                return false;
        }
        dt.year += 2000;
        uint32_t seconds = 0;
        RTC_HAL_ConvertDatetimeToSecs(&dt, &seconds);
        int tz_seconds = tz * 15 * 60;
        if(tz_seconds < 0) {
          seconds += (uint32_t)(abs(tz_seconds));
        } else {
          seconds -= tz_seconds;
        }
        RTC_HAL_ConvertSecsToDatetime(&seconds, &dt);
        return true;
    }
    return false;
}

int Hologram::getChargeState() {
    int charge = 8;
    if(modem_state > MODEM_STATE_SHUTDOWN) {
        if(modem.query("+HCHARGE") == MODEM_OK)
            sscanf(modem.lastResponse(), "+HCHARGE: %d", &charge);
    }
    return charge;
}

String Hologram::systemVersion() {
    char ver[9];
    int value=0;
    if(modem_state > MODEM_STATE_SHUTDOWN) {
        if(modem.set("+HSYS", "2") == MODEM_OK) {
            if(sscanf(modem.lastResponse(), "+HSYS: %d,\"%[^\"]\"", &value, ver) == 2) {
                if(value == 2)
                    return String(ver);
            }
        }
    }
    return String("0.0.0");
}

String Hologram::getICCID() {
    if(modem_state != MODEM_STATE_DISCONNECTED) {
        powerUp();
        if(modem.query("+CCID") == MODEM_OK) {
            if(strncmp(modem.lastResponse(), "+CCID: ", 7) == 0) {
                return String(&(modem.lastResponse()[7]));
            }
        }
    }
    return String("Not available");
}

String Hologram::getIMEI() {
    if(modem_state != MODEM_STATE_DISCONNECTED) {
        powerUp();
        if(modem.command("+CIMI") == MODEM_OK) {
            return String(&(modem.lastResponse()[0]));
        }
    }
    return String("Not available");
}

String Hologram::getNetworkOperator() {
    if(modem_state != MODEM_STATE_DISCONNECTED) {
        powerUp();
        if(modem.set("+UDOPN", "12") == MODEM_OK) {
            if(strncmp(modem.lastResponse(), "+UDOPN: 12,\"", 12) == 0) {
                String s = String(&(modem.lastResponse()[12]));
                s.remove(s.length()-1);
                return s;
            }
        }
    }
    return String("Not available");
}

bool Hologram::getLocation(int accuracy, int maxseconds) {
    //AT+ULOC=2,2,0,360,10
    //AT+HLOC=360,10
    if(modem_state == MODEM_STATE_DISCONNECTED) return false;
    powerUp();
    modem.startSet("+HLOC");
    modem.appendSet(maxseconds);
    modem.appendSet(",");
    modem.appendSet(accuracy);
    return modem.completeSet() == MODEM_OK;
}

bool Hologram::setRGB(String name) {
    if(modem_state > MODEM_STATE_SHUTDOWN) {
        return modem.set("+HRGBN", name.c_str()) == MODEM_OK;
    }
    return false;
}

bool Hologram::setRGB(int hexcode) {
    if(modem_state > MODEM_STATE_SHUTDOWN) {
        String h(hexcode, 16);
        return modem.set("+HRGBH", h.c_str()) == MODEM_OK;
    }
    return false;
}

bool Hologram::setLED(bool on) {
    if(modem_state > MODEM_STATE_SHUTDOWN) {
        return modem.set("+HLED", on ? "1" : "0") == MODEM_OK;
    }
    return false;
}

void Hologram::powerUp() {
    int retries = 30;
    int delayinc = 0;
    if(modem_state == MODEM_STATE_READY) {
        return;
    } else if(modem_state == MODEM_STATE_SHUTDOWN) {
        protocol_version = 0;
        modem.command("", 150); //pulse an AT but don't expect a response
        modem_state = MODEM_STATE_UNKNOWN;
    } else if(modem_state == MODEM_STATE_DISCONNECTED) {
        if(modem.command("+HCONNECT") == MODEM_OK)
            modem_state = MODEM_STATE_READY;
    }
    while(protocol_version == 0) {
        modem.checkURC();
        if(modem.command("", 100, retries) != MODEM_OK) {
            resetSystem();
            Dash.snooze(1000+(delayinc*2000));
            retries *= 2;
            delayinc++;
        } else if(protocol_version == 0) {
            if(modem.query("+HOLO", 100, 10) == MODEM_OK) {
                sscanf(modem.lastResponse(), "+HOLO: %d", &protocol_version);
            } else {
                resetSystem();
                Dash.snooze(3000);
                modem_state = MODEM_STATE_UNKNOWN;
            }
        }
        break;
    }

    modem_state = MODEM_STATE_READY;
}

void Hologram::powerDown() {
    pollEvents();
    modem_state = MODEM_STATE_SHUTDOWN;
    modem.command("+HSHUTDOWN");
    protocol_version = 0;
}

void Hologram::notifySMS() {
    if(sms_pending) {
        if(sms_callback)
            sms_callback(String(sms_sender), sms_dt, String(sms_message));
        sms_pending = false;
    }
}

void Hologram::pollEvents() {
    //check SMS pending...
    if(ready) {
        notifySMS(); //clear any SMS already received
        modem.checkURC();
        checkIncoming();
        while(checkSMS() > 0) {
            if(modem.command("+HSMSRD") == MODEM_OK) {
                uint32_t startMillis = millis();
                while (!sms_pending && (millis() - startMillis < 1000)) {
                    modem.checkURC();
                }
                if(!sms_pending) break;
                notifySMS();
            } else {
                break;
            }
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

bool Hologram::attachTopic(const char* topic) {
    if(strlen(topic) > MAX_TOPIC_SIZE) return false;
    resetBuffer();
    if(num_topics == MAX_TOPICS) return false;
    strcpy(topics[num_topics++], topic);
    return true;
}

void Hologram::clear() {
    num_topics = 0;
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
    if(modem_state == MODEM_STATE_DISCONNECTED) return sendFinalize(false);;
    powerUp();

    if(modem.command("+HMRST") != MODEM_OK)
        return sendFinalize(false);

    for(int i=0; i<num_topics; i++) {
        if(protocol_version >= 2)
            modem.set("+HTOPIC", topics[i]);
        else
            modem.set("+HTAG", topics[i]);
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

bool Hologram::sendMessage(const String &content, const String &topic) {
    return sendMessage(content.c_str(), topic.c_str());
}

bool Hologram::sendMessage(const char* content, const char* topic) {
    return sendMessage((const uint8_t*)content, strlen(content), topic);
}

bool Hologram::sendMessage(const String &content, const char* topic) {
    return sendMessage(content.c_str(), topic);
}

bool Hologram::sendMessage(const char* content, const String &topic) {
    return sendMessage(content, topic.c_str());
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

bool Hologram::sendMessage(const uint8_t* content, uint32_t length, const String &topic) {
    return sendMessage(content, length, topic.c_str());
}

bool Hologram::sendMessage(const uint8_t* content, uint32_t length, const char* topic) {
    if(topic)
        attachTopic(topic);
    return sendMessage(content, length);
}

ArduinoModem modem;
