/*
  DashReadEvalPrint.cpp - Dash commands for the Read-Eval-Print-Loop.

  http://hologram.io

  Copyright (c) 2016 Konekt, Inc.  All rights reserved.

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

#include "DashReadEvalPrint.h"
#include "hal/fsl_rtc_hal.h"

DashCloudProvider DashCloud;
DashLEDProvider DashLED;
DashFunctionsProvider DashFunctions;
DashTimerProvider DashTimer;
DashClockProvider DashClock;
DashChargerProvider DashCharger;
DashModemProvider DashModem;

void DashReadEvalPrintLoop::begin()
{
    setPrompt("Dash>");
    addProvider(DashCloud);
    addProvider(DashLED);
    addProvider(DashFunctions);
    addProvider(DashTimer);
    addProvider(DashClock);
    addProvider(DashCharger);
    addProvider(DashModem);
}

void DashReadEvalPrintLoop::sleep()
{
    port->waitToEmpty();
    Dash.sleep();
}

void DashReadEvalPrintLoop::worker()
{
    HologramCloud.pollEvents();
}

void DashReadEvalPrintLoop::setDebugPort(HardwareSerial &debug)
{
    debugport = &debug;
}

DashReadEvalPrintLoop DashReadEvalPrint;

static const ReadEvalPrintCommand CLOUD[] = {
    {2, 0, "connect to the cloud",                      "cloud", "connect"},                        //0
    {2, 0, "disconnect from the cloud",                 "cloud", "disconnect"},                     //1
    {3, 7, "send a message to the Hologram Cloud",      "cloud", "send", "<message>", "[topics]"},  //2
    {2, 0, "get signal strength",                       "cloud", "signal"},                         //3
    {2, 0, "get connect state",                         "cloud", "status"},                         //4
    {2, 0, "wake up the modem",                         "cloud", "on"},                             //5
    {2, 0, "shutdown the modem",                        "cloud", "off"},                            //6
    {2, 0, "cloud version number",                      "cloud", "version"},                        //7
    {2, 0, "ICCID of the SIM card",                     "cloud", "iccid"},                          //8
    {2, 0, "IMEI of the SIM card",                      "cloud", "imei"},                           //9
    {2, 0, "current network operator",                  "cloud", "operator"},                       //10
    {3, 0, "start inbound connection listener",         "cloud", "listen", "<port>"},               //11
    {2, 0, "get approximate location",                  "cloud", "location"},                       //12
    {2, 0, "reset the system processor",                "cloud", "reset"},                          //13
    {3, 0, "turn on the System LED",                    "cloud", "led", "on"},                      //14
    {3, 0, "turn off the System LED",                   "cloud", "led", "off"},                     //15
};

const ReadEvalPrintCommand* DashCloudProvider::getTable(uint32_t *num_commands)
{
    *num_commands = sizeof(CLOUD)/sizeof(ReadEvalPrintCommand);
    return CLOUD;
}

void DashCloudProvider::printStatus(Print &port) {
    switch(HologramCloud.getConnectionStatus()) {
        case CLOUD_REGISTERED: port.println("Registered."); break;
        case CLOUD_CONNECTED: port.println("Connected."); break;
        case CLOUD_ERR_SIM: port.println("Error. Check SIM card inserted properly and power cycle."); break;
        case CLOUD_ERR_UNREGISTERED: port.println("Not registered on network."); break;
        case CLOUD_ERR_MODEM_OFF: port.println("Modem disconnected."); break;
        case CLOUD_ERR_SIGNAL: port.println("No signal. Check antenna."); break;
        case CLOUD_ERR_CONNECT: port.println("No connection. Check if SIM is active."); break;
        default: port.print("Unknown ");break;
    }
}

bool DashCloudProvider::event(ReadEvalPrintEvent &event, Print &port)
{
    switch(event.commandIndex()) {
    case 0: //cloud connect
        port.print("Connecting... ");
        if (HologramCloud.connect()) {
            break;
        }
        port.println("Failed");
        printStatus(port);
        break;
    case 1: //cloud disconnect
        port.print("Disconnecting... ");
        if (HologramCloud.disconnect()) {
            break;
        }
        port.println("Failed");
        printStatus(port);
    case 2: //cloud send
    {
        uint32_t numtopics = event.numArguments() - 3;
        for (int i = 0; i < numtopics; i++) {
            HologramCloud.attachTopic(event.getArgument(3 + i));
        }
        port.print("Sending message... ");
        if (HologramCloud.sendMessage(event.getArgument(2))) {
            port.println("Complete");
        } else {
            port.println("Failed");
        }
        break;
    }
    case 3: //cloud signal
        port.print("Signal Strength: ");
        port.println(HologramCloud.getSignalStrength());
        break;

    case 4: //cloud status
        printStatus(port);
        break;

    case 5: //cloud on
        port.print("Waking up... ");
        HologramCloud.powerUp();
        port.println("Done");
        break;
    case 6: //cloud off
        port.print("Shutting Down... ");
        HologramCloud.powerDown();
        port.println("Done");
        break;

    case 7: //cloud version
        port.println(HologramCloud.systemVersion());
        break;

    case 8: //cloud iccid
        port.println(HologramCloud.getICCID());
        break;

    case 9: //cloud imei
        port.println(HologramCloud.getIMEI());
        break;

    case 10: //cloud operator
        port.println(HologramCloud.getNetworkOperator());
        break;

    case 11: //cloud listen port
        {
            int portnum = 0;
            if(event.tolong(2, &portnum)) {
                if(HologramCloud.listen(portnum)) {
                    port.print("Listening on port ");
                    port.println(portnum);
                } else {
                    port.println("Listening failed");
                }
            } else {
                return event.invalidParameter(2, "port number");
            }
        }
        break;

    case 12: //cloud location
        {
            if(HologramCloud.getLocation()) {
                port.println("Location requested. Response can take a few minutes.");
            } else {
                port.println("Location not available.");
            }
        }
        break;
    case 13: //cloud reset
        HologramCloud.resetSystem();
        break;
    case 14: //cloud led on
        if(HologramCloud.setLED(true)) {
            port.println("LED on");
        } else {
            port.println("Failed");
        }
        break;
    case 15: //cloud led off
        if(HologramCloud.setLED(false)) {
            port.println("LED off");
        } else {
            port.println("Failed");
        }
        break;
    default:
        return false;
    }
    return true;
}

static const ReadEvalPrintCommand MODEM[] = {
    {3, 0, "send a command to the modem",               "modem", "cmd", "<command>"},               //0
    {3, 0, "send a query to the modem",                 "modem", "query", "<command>"},             //1
    {4, 0, "set a value on the modem",                  "modem", "set", "<command>", "<value>"},    //2
    {3, 0, "enter passthrough mode if command is 1",    "modem", "passthrough", "<command>"},       //3
    {2, 0, "no response",                               "debug", "timeout"},                        //4
    {3, 0, "OK response after delay",                   "debug", "delay", "<value>"},               //5
    {3, 0, "ERROR response after delay",                "debug", "delayerr", "<value>"},            //6
    {4, 0, "read memory",                               "mem", "read", "<address>", "<size>"},      //7
};

const ReadEvalPrintCommand* DashModemProvider::getTable(uint32_t *num_commands)
{
    *num_commands = sizeof(MODEM)/sizeof(ReadEvalPrintCommand);
    return MODEM;
}

bool DashModemProvider::event(ReadEvalPrintEvent &event, Print &port)
{
    switch(event.commandIndex()) {
    case 0: //modem cmd
        if(modem.command(event.getArgument(2)) == MODEM_OK) {
            if(strlen(modem.lastResponse()) > 0) {
                port.println(modem.lastResponse());
                port.println();
            }
            port.println("OK");
        } else {
            port.println("ERROR");
        }
        break;
    case 1: //modem query
        if(modem.query(event.getArgument(2)) == MODEM_OK) {
            if(strlen(modem.lastResponse()) > 0) {
                port.println(modem.lastResponse());
                port.println();
            }
            port.println("OK");
        } else {
            port.println("ERROR");
        }
        break;
    case 2: //modem set
        if(modem.set(event.getArgument(2), event.getArgument(3)) == MODEM_OK) {
            if(strlen(modem.lastResponse()) > 0) {
                port.println(modem.lastResponse());
                port.println();
            }
            port.println("OK");
        } else {
            port.println("ERROR");
        }
        break;
    case 3: //modem passthrough
    {
        int val = 0;
        if(!event.tolong(2, &val) || val != 1) {
            return event.invalidParameter(2, "must be 1 to enter passthrough");
        }
        if(HologramCloud.enterPassthrough()) {
            port.println("Entering passthrough, reset to exit");
            HardwareSerial *s = (HardwareSerial *)&port;
            while(1) {
                while(s->available())
                  SerialSystem.write(s->read());
                while(SerialSystem.available())
                  s->write(SerialSystem.read());
            }
        } else {
            port.println("Passthrough not available");
        }
        break;
    }
    case 4:
        {
            port.print("Calling timeout... ");
            int ret = HologramCloud.sendTimeout();
            port.println(ret);
        }
        break;
    case 5:
        {
            port.print("Calling delay... ");
            int to = 0;
            event.tolong(2, &to);
            int ret = HologramCloud.sendDelay(true, to);
            port.println(ret);
        }
        break;
    case 6:
        {
            port.print("Calling delay error... ");
            int to;
            event.tolong(2, &to);
            int ret = HologramCloud.sendDelay(false, to);
            port.println(ret);
        }
        break;
    case 7:
        {
            int address, size;
            event.tolong(2, &address);
            event.tolong(3, &size);
            
            for(int i=0; i<size; i++) {
                uint8_t b = *(uint8_t*)(address+i);
                if(b < 0x10) port.write('0');
                port.print(b, HEX);
                port.write(' ');
            }
            port.println();
        }
        break;
    default:
        return false;
    }
    return true;
}

static const ReadEvalPrintCommand LED[] = {
    {1, 0, "print LED status",                      "led"},
    {2, 0, "turn on LED",                           "led", "on"},
    {2, 0, "turn off LED",                          "led", "off"},
    {2, 0, "switch LED (on->off or off->on)",       "led", "toggle"},
    {3, 0, "set LED brightness to percent (1-100)", "led", "dim", "<percent>"},
    {4, 0, "LED blinks on ms, off ms",              "led", "pulse", "<on>", "<off>"},
    {3, 0, "RGB HTML color name or 0xRRGGBB",       "rgb", "on", "<color>"},
    {2, 0, "Turn off RGB",                          "rgb", "off"},
};

const ReadEvalPrintCommand* DashLEDProvider::getTable(uint32_t *num_commands)
{
    *num_commands = sizeof(LED)/sizeof(ReadEvalPrintCommand);
    return LED;
}

void DashLEDProvider::printLED(Print &port)
{
    uint32_t on, off, dim;
    Dash.stateLED(&on, &off, &dim);

    port.print("led ");
    if(on || off) {
        port.print("pulse ");
        port.print(on/(SystemBusClock/1000));
        port.print("ms/");
        port.print(off/(SystemBusClock/1000));
        port.println("ms");
    } else if(dim) {
        port.print(dim);
        port.println("%");
    } else {
        port.println(digitalRead(DASH_LED) == HIGH ? "on" : "off");
    }
}

bool DashLEDProvider::event(ReadEvalPrintEvent &event, Print &port)
{
    switch(event.commandIndex()) {
    case 0: break;
    case 1: Dash.onLED(); break;
    case 2: Dash.offLED(); break;
    case 3: Dash.toggleLED(); break;
    case 4:
        {   //LED DIM
            int pct = 0;
            if(event.tolong(2, &pct) && pct >= 0 && pct <= 100) {
                Dash.dimLED(pct);
            } else {
                return event.invalidParameter(2, "(0-100)");
            }
            break;
        }
    case 5:
        {   //LED PULSE
            int on, off;
            if(event.tolong(2, &on) && on >= 0) {
                if(event.tolong(3, &off) && off >= 0) {
                    Dash.pulseLED(on, off);

                } else {
                    return event.invalidParameter(3);
                }
            } else {
                return event.invalidParameter(2);
            }
        }
        break;
    case 6:
        {
            int v;
            if(event.tolong(2, &v)) {
                if(HologramCloud.setRGB(v))
                    return true;
            } else if(HologramCloud.setRGB(event.getArgument(2))) {
                    return true;
            }
            return event.invalidParameter(2);
        }
    case 7:
        if(HologramCloud.offRGB()) {
            return true;
        } else {
            return event.invalidParameter(0);
        }
    default:
        return false;
    }

    printLED(port);
    return true;
}

static const ReadEvalPrintCommand FUNCTIONS[] = {
    {2, 0, "go into Deep Sleep",                    "dash", "deepsleep"},                           //0
    {2, 0, "go into Shutdown",                      "dash", "shutdown"},                            //1
    {3, 0, "boot version number",                   "dash", "boot", "version"},                     //2
    {2, 0, "Dash repl version",                     "repl", "version"},                             //3
    {3, 0, "erase entire data flash",               "flash", "erase", "all"},                       //4
    {3, 0, "erase 4096-byte flash sector",          "flash", "erase", "<address>"},                 //5
    {4, 0, "read flash",                            "flash", "read", "<address>", "<size>"},        //6
    {4, 7, "write up to 8 hex bytes to flash",      "flash", "write", "<address>", "<hex>"},        //7
    {3, 0, "print string from flash",               "flash", "print", "<address>"},                 //8
    {4, 0, "write string to flash",                 "flash", "string", "<address>", "<string>"},    //9
};

const ReadEvalPrintCommand* DashFunctionsProvider::getTable(uint32_t *num_commands)
{
    *num_commands = sizeof(FUNCTIONS)/sizeof(ReadEvalPrintCommand);
    return FUNCTIONS;
}

bool DashFunctionsProvider::event(ReadEvalPrintEvent &event, Print &port)
{
    switch(event.commandIndex()) {
    case 0:
        Dash.deepSleep();
        break;
    case 1:
        Dash.shutdown();
        break;
    case 2:
        port.println(Dash.bootVersion());
        break;
    case 3:
        port.print("Dash REPL version ");
        port.println(DASH_REPL_VERSION);
        break;
    case 4:
        port.print("Erasing data flash...");
        port.println(DashFlash.eraseAll() == true ? "Complete" : "Failed");
        break;
    case 5:
        {
            port.print("Erasing sector...");
            int address;
            event.tolong(2, &address);
            port.println(DashFlash.eraseSector(address) == true ? "Complete" : "Failed");
        }
        break;
    case 6: //read flash hex
        {
            int address, size;
            event.tolong(2, &address);
            event.tolong(3, &size);
            if(!DashFlash.beginRead(address)) return event.invalidParameter(2);
            
            for(int i=0; i<size; i++) {
                uint8_t b = DashFlash.continueRead();
                if(b < 0x10) port.write('0');
                port.print(b, HEX);
                port.write(' ');
            }
            port.println();
        }
        break;
    case 7: //write flash hex
        {
            int address;
            event.tolong(2, &address);
            if(!DashFlash.beginWrite(address)) return event.invalidParameter(2);
            int count = event.numArguments() - 3;
            for(int i=0; i<count; i++) {
                uint8_t data;
                if(sscanf(event.getArgument(3+i), "%x", &data) == 1) {
                    DashFlash.continueWrite(data);
                }
                else {
                    return event.invalidParameter(i+3);
                }
            }
            DashFlash.endWrite();
            port.println("Complete");
        }
        break;
    case 8: //print flash
        {
            int address;
            event.tolong(2, &address);
            if(!DashFlash.beginRead(address)) return event.invalidParameter(2);
            
            while(true) {
                uint8_t c = DashFlash.continueRead();
                if(c < 0x20 || c >= 0x7F) break;
                port.print((char)c);
            }
            port.println();
        }
        break;
    case 9: //write string to flash
        {
            int address;
            event.tolong(2, &address);
            const char* str = event.getArgument(3);
            if(DashFlash.writeString(address, str))
                port.println("Complete");
            else
                port.println("Failed");
        }
        break;
    default:
        return false;
    }

    return true;
}

static const ReadEvalPrintCommand TIMER[] = {
    {3, 0, "start a one-shot timer",                "timer", "oneshot", "<ms>"},
    {3, 0, "start a repeating timer",               "timer", "repeat", "<ms>"},
    {2, 0, "stop the timer",                        "timer", "stop"},
};

const ReadEvalPrintCommand* DashTimerProvider::getTable(uint32_t *num_commands)
{
    *num_commands = sizeof(TIMER)/sizeof(ReadEvalPrintCommand);
    return TIMER;
}

bool DashTimerProvider::event(ReadEvalPrintEvent &event, Print &port)
{
    bool repeat = true;

    switch(event.commandIndex()) {
    case 0: //one-shot
        repeat = false;
        //fallthrough
    case 1: //repeat
        {
            int ms;
            if(event.tolong(2, &ms)) {
                Dash.startTimerMS(ms, repeat);
                port.println("Timer started");
            } else {
                return event.invalidParameter(2);
            }
            break;
        }
    case 2: //stop
        Dash.stopTimer();
        port.println("Timer Stopped");
        break;
    default:
        return false;
    }

    return true;
}

static const ReadEvalPrintCommand CLOCK[] = {
    {1, 0, "print the current date/time",       "clock"},                                                       //0
    {2, 6, "set the clock",                     "clock", "set", "[y]", "[mon]", "[d]", "[h]", "[min]", "[s]"},  //1
    {2, 0, "adjusted ticks per second",         "clock", "adjust"},                                             //2
    {3, 0, "adjust the ticks per second",       "clock", "adjust", "<offset>"},                                 //3
    {2, 0, "check if the clock is running",     "clock", "running"},                                            //4
    {2, 0, "check if the clock was reset",      "clock", "reset"},                                              //5
    {2, 0, "synchronize with network time",     "clock", "sync"},                                               //6
    {2, 0, "synchronize with UTC",              "clock", "utc"},                                                //7
    {1, 0, "print the alarm state",             "alarm"},                                                       //8
    {2, 0, "cancel the current alarm",          "alarm", "cancel"},                                             //9
    {2, 6, "set the alarm",                     "alarm", "set", "[y]", "[mon]", "[d]", "[h]", "[min]", "[s]"},  //10
    {3, 0, "set the alarm <seconds> from now",  "alarm", "seconds", "<seconds>"},                               //11
    {3, 0, "set the alarm <minutes> from now",  "alarm", "minutes", "<minutes>"},                               //12
    {3, 0, "set the alarm <hours> from now",    "alarm", "hours", "<hours>"},                                   //13
    {3, 0, "set the alarm <days> from now",     "alarm", "days", "<days>"},                                     //14

};

const ReadEvalPrintCommand* DashClockProvider::getTable(uint32_t *num_commands) {
    *num_commands = sizeof(CLOCK)/sizeof(ReadEvalPrintCommand);
    return CLOCK;
}

void DashClockProvider::printCurrentDateTime(Print &port) {
    port.println(Clock.currentDateTime());
}

bool DashClockProvider::parseDateTime(ReadEvalPrintEvent &event, rtc_datetime_t &dt) {
    int v=0;
    rtc_datetime_t now;
    Clock.getDateTime(now);
    dt.year = 1970;
    dt.month = 1;
    dt.day = 1;
    dt.hour = 0;
    dt.minute = 0;
    dt.second = 0;
    uint16_t * pdt = (uint16_t*)&dt;
    uint16_t * pnow = (uint16_t*)&now;

    for(int field=0; field<6; field++) {
        int arg = field+2;
        if(event.numArguments() > arg) {
            if(event.tolong(arg, &v)) {
                pdt[field] = (uint16_t)v;
            } else if(strcmp(event.getArgument(arg), ".") == 0) {
                pdt[field] = pnow[field];
            } else {
                return event.invalidParameter(arg);
            }
            if(!RTC_HAL_IsDatetimeCorrectFormat(&dt)) {
                return event.invalidParameter(arg);
            }
        } else {
            break;
        }
    }
    return true;
}

bool DashClockProvider::event(ReadEvalPrintEvent &event, Print &port) {
    int v=0;
    int alarm_mult = 1;
    rtc_datetime_t dt;

    switch(event.commandIndex()) {
    case 0: printCurrentDateTime(port); break;

    case 1: //DATE/TIME set
        if(parseDateTime(event, dt)) {
            Clock.setDateTime(dt);
            printCurrentDateTime(port);
        } else {
            return false;
        }
        break;

    case 3:
        if(event.tolong(2, &v)) {
            RTC_HAL_SetTimeCompensationRegister(RTC, (uint8_t)(v&0xFF));
        } else {
            return event.invalidParameter(2);
        }
        //Fallthrough
    case 2:
        port.print("Clock adjusted: ");
        port.println(Clock.adjusted());
        break;

    case 4:
        port.print("Clock is ");
        if(!Clock.isRunning()) port.print("not ");
        port.println("running");
        break;
    case 5:
        port.print("Clock was ");
        if(!Clock.wasReset()) port.print("not ");
        port.println("reset");
        break;

    case 6:
        //clock sync
        if(HologramCloud.getNetworkTime(dt)) {
            Clock.setDateTime(dt);
            port.println(Clock.currentDateTime());
        } else {
            port.println("Network time not available");
        }
        break;
    case 7:
        //clock utc
        if(HologramCloud.getUTC(dt)) {
            Clock.setDateTime(dt);
            port.println(Clock.currentDateTime());
        } else {
            port.println("Network time not available");
        }
        break;
    case 8: //alarm
        if(Clock.alarmExpired()) {
            port.println("Alarm expired");
        } else {
            int seconds = RTC_HAL_GetAlarmReg(RTC) - RTC_HAL_GetSecsReg(RTC);
            if(seconds < 0) {
                port.println("Alarm invalid");
            } else {
                int hours = seconds / 3600;
                seconds -= hours*3600;
                int minutes = seconds / 60;
                seconds -= minutes*60;
                port.print(hours);
                port.print(":");
                if(minutes < 10) port.print("0");
                port.print(minutes);
                port.print(":");
                if(seconds < 10) port.print("0");
                port.print(seconds);
                port.println(" remaining");
            }
        }
        break;

    case 9:
        Clock.cancelAlarm();
        port.println("Alarm canceled");
        break;

    case 10: //Alarm set
        if(parseDateTime(event, dt)) {
            if(!Clock.setAlarm(dt)) {
                return event.invalidParameter(2, "Alarm cannot be in the past");
            }
            port.println("Alarm set");
        } else {
            return false;
        }
        break;

    //ALARM
    case 14: alarm_mult*=24; //alarm days
    case 13: alarm_mult*=60; //alarm hours
    case 12: alarm_mult*=60; //alarm minutes
    case 11: { //set alarm seconds
        if(event.tolong(2, &v)) {
            Clock.setAlarmSecondsFromNow(v*alarm_mult);
        } else {
            return event.invalidParameter(2);
        }
        port.println("Alarm set");
    }
    break;

    default:
        return false;
    }

    return true;
}


static const ReadEvalPrintCommand CHARGER[] = {
    {2, 0, "print the battery percent remaining",       "battery", "percent"},
    {2, 0, "print the battery voltage in mV",           "battery", "voltage"},
    {2, 0, "print battery status",                      "battery", "status"},
    {1, 0, "print if charger is enabled/disabled",      "charger"},
    {2, 0, "enable/disable the charger (if supported)", "charger", "<enable/disable>"},
    {2, 0, "quickstart the fuel gauge",                 "fuel", "quickstart"},
    {2, 0, "restart the fuel gauge",                    "fuel", "restart"},
    {2, 0, "put the fuel gauge to sleep",               "fuel", "sleep"},
    {2, 0, "wake the fuel gauge from sleep",            "fuel", "wake"},

};

const ReadEvalPrintCommand* DashChargerProvider::getTable(uint32_t *num_commands)
{
    *num_commands = sizeof(CHARGER)/sizeof(ReadEvalPrintCommand);
    return CHARGER;
}

bool DashChargerProvider::event(ReadEvalPrintEvent &event, Print &port)
{
    switch(event.commandIndex()) {
    case 0:
        port.print("Battery ");
        port.print(Charger.batteryPercentage());
        port.println("%");
        break;
    case 1:
        port.print("Battery ");
        port.print(Charger.batteryMillivolts());
        port.println("mV");
        break;
    case 2:
        switch(HologramCloud.getChargeState()) {
        case 0:
            port.println("Charge Fault!!!");
            break;
        default:
        case 1:
        case 5:
            port.println("Charge State Invalid!!!");
            break;
        case 2:
            port.println("Charging");
            break;
        case 3:
            port.println("Low battery");
            break;
        case 4:
            port.println("Charged");
            break;
        case 6:
            port.println("No battery");
            break;
        case 7:
            port.println("No input power");
            break;
        }
        break;
    case 4:
        if(strcmp(event.getArgument(1), "enable") == 0) {
            Charger.enable(true);
        } else if(strcmp(event.getArgument(1), "disable") == 0) {
            Charger.enable(false);
        } else {
            return event.invalidParameter(1, "must be 'enable' or 'disable'");
        }
        //fallthrough
    case 3:
        port.print("Charger ");
        if(Charger.isControllable())
            port.println(Charger.isEnabled() ? "Enabled" : "Disabled");
        else
            port.println("is not controllable");
        break;
    case 5:
        FuelGauge.quickStart();
        port.println("Fuel Gauge Quickstart");
        break;
    case 6:
        FuelGauge.reset();
        port.println("Fuel Gauge Reset");
        break;
    case 7:
        FuelGauge.sleep();
        port.println("Fuel Gauge in Sleep");
        break;
    case 8:
        FuelGauge.wake();
        port.println("Fuel Gauge Awake");
        break;
    default:
        return false;
    }

    return true;
}
