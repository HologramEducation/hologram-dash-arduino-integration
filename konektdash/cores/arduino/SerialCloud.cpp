/*
  SerialCloud.cpp - Virutal serial port interface to HologramCloud

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

#include "SerialCloud.h"
#include "delay.h"

extern Hologram HologramCloud;
extern SerialCloudClass SerialCloud;

void sms_receiver(const String &sender, const rtc_datetime_t &timestamp, const String &message) {
    SerialCloud.pushSMS(message);
}

void SerialCloudClass::begin(unsigned long baudrate) {
    begin(baudrate, 0);
}
void SerialCloudClass::begin(unsigned long baudrate, uint16_t config) {
    HologramCloud.attachHandlerSMS(sms_receiver);
}

void SerialCloudClass::end() {
    rxBuffer.clear();
}

size_t SerialCloudClass::write(uint8_t x) {
    size_t s = HologramCloud.write(x);
    if(x == '\n') {
        while(!HologramCloud.isConnected()) {
            delay(100);
        }
        bool success = false;
        while(!success) {
            success = HologramCloud.sendMessage();
            if(success) {
                store("+EVENT:MSGSENT\r\n");
            } else {
                delay(100);
            }
        }
        
        HologramCloud.clear();
    }
    return s;
}

void SerialCloudClass::store(const char* str) {
    for(int i=0; i<strlen(str); i++) {
        rxBuffer.store_char(str[i]);
    }
}

void SerialCloudClass::store(int i) {
    char buffer[12];
    sprintf(buffer, "%d", i);
    store(buffer);
}

void SerialCloudClass::pushSMS(const String &message) {
    store("+EVENT:SMSRCVD,");
    store(message.length());
    rxBuffer.store_char(',');
    store(message.c_str());
    rxBuffer.store_char('\r');
    rxBuffer.store_char('\n');
}

int SerialCloudClass::available()
{
    return rxBuffer.available();
}

int SerialCloudClass::peek()
{
    return rxBuffer.peek();
}

int SerialCloudClass::read()
{
    return rxBuffer.read_char();
}

void SerialCloudClass::flush()
{
    rxBuffer.clear();
}
