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
#pragma once

#include <ReadEvalPrint.h>

#define DASH_REPL_VERSION "0.11.1"

class DashReadEvalPrintLoop : public ReadEvalPrintLoop
{
public:
    void begin();
    void setDebugPort(HardwareSerial &debug);
protected:
    virtual void sleep();
    virtual void worker();
    HardwareSerial *debugport;
};

extern DashReadEvalPrintLoop DashReadEvalPrint;

class DashCloudProvider : public ReadEvalPrintProvider
{
public:
    virtual const ReadEvalPrintCommand* getTable(uint32_t *num_commands);
    virtual bool event(ReadEvalPrintEvent &event, Print &port);
    virtual const char* getHelpHeader() {return "Hologram Cloud";}
protected:
    void printStatus(Print &port);
};

class DashModemProvider : public ReadEvalPrintProvider
{
public:
    virtual const ReadEvalPrintCommand* getTable(uint32_t *num_commands);
    virtual bool event(ReadEvalPrintEvent &event, Print &port);
    virtual const char* getHelpHeader() {return "Modem Commands (Advanced Usage Only)";}
    virtual bool advanced() {return true;}
};

class DashLEDProvider : public ReadEvalPrintProvider
{
public:
    virtual const ReadEvalPrintCommand* getTable(uint32_t *num_commands);
    virtual bool event(ReadEvalPrintEvent &event, Print &port);
    virtual const char* getHelpHeader() {return "Dash LED";}
protected:
    void printLED(Print &port);
};

class DashFunctionsProvider : public ReadEvalPrintProvider
{
public:
    virtual const ReadEvalPrintCommand* getTable(uint32_t *num_commands);
    virtual bool event(ReadEvalPrintEvent &event, Print &port);
    virtual const char* getHelpHeader() {return "Dash Commands";}
};

class DashTimerProvider : public ReadEvalPrintProvider
{
public:
    virtual const ReadEvalPrintCommand* getTable(uint32_t *num_commands);
    virtual bool event(ReadEvalPrintEvent &event, Print &port);
    virtual const char* getHelpHeader() {return "Timer";}
};

class DashClockProvider : public ReadEvalPrintProvider
{
public:
    virtual const ReadEvalPrintCommand* getTable(uint32_t *num_commands);
    virtual bool event(ReadEvalPrintEvent &event, Print &port);
    virtual const char* getHelpHeader() {return "Clock and Alarm";}
protected:
    void printCurrentDateTime(Print &port);
    bool parseDateTime(ReadEvalPrintEvent &event, rtc_datetime_t &dt);
};

class DashChargerProvider : public ReadEvalPrintProvider
{
public:
    virtual const ReadEvalPrintCommand* getTable(uint32_t *num_commands);
    virtual bool event(ReadEvalPrintEvent &event, Print &port);
    virtual const char* getHelpHeader() {return "Battery and Charger";}
};
