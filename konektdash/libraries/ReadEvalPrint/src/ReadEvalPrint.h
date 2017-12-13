/*
  ReadEvalPrint.h - processes commands in a Read-Eval-Print-Loop.

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

#include "Arduino.h"
#include "HardwareSerial.h"

#ifndef READEVALPRINT_MAX_PROVIDERS
#define READEVALPRINT_MAX_PROVIDERS 20
#endif

#ifndef READEVALPRINT_MAX_ARGS
#define READEVALPRINT_MAX_ARGS 10
#endif

typedef struct
{
    uint16_t req;
    uint16_t opt;
    const char *help;
    const char *argv[READEVALPRINT_MAX_ARGS];
}ReadEvalPrintCommand;

class ReadEvalPrintEvent
{
public:
    virtual int commandIndex()=0;
    virtual int numArguments()=0;
    virtual const char * getArgument(int arg)=0;
    virtual bool invalidParameter(int arg, const char* msg="") = 0;
    virtual bool tolong(int arg, int *value) = 0;
};

class ReadEvalPrintProvider
{
public:
    virtual const ReadEvalPrintCommand* getTable(uint32_t *num_commands) = 0;
    virtual bool event(ReadEvalPrintEvent &event, Print &port){return false;}
    virtual const char* getHelpHeader() {return "";}
    virtual bool advanced() {return false;}
};

typedef bool (*ReadEvalPrintEventHandler)(ReadEvalPrintEvent &event, Print &port);

class ReadEvalPrintBaseProvider : public ReadEvalPrintProvider
{
public:
    ReadEvalPrintBaseProvider(const ReadEvalPrintCommand *commands, uint32_t num_commands, ReadEvalPrintEventHandler handler, const char* help_header=NULL);
    const ReadEvalPrintCommand* getTable(uint32_t *num_commands);
    bool event(ReadEvalPrintEvent &event, Print &port);
    virtual const char* getHelpHeader() {if(header) return header; return "";}
protected:
    const ReadEvalPrintCommand *commands;
    uint32_t command_count;
    ReadEvalPrintEventHandler handler;
    const char* header;
};

class SerialMirror : public HardwareSerial
{
public:
    SerialMirror();
    bool add(HardwareSerial &serialPort);
    virtual void begin(unsigned long);
    virtual void begin(unsigned long baudrate, uint16_t config);
    virtual void end();
    virtual int available(void);
    virtual int peek(void);
    virtual int read(void);
    virtual void flush(void);
    virtual size_t write(uint8_t);
    using Print::write; // pull in write(str) and write(buf, size) from Print
    virtual operator bool() { return true; }
protected:
    int portcount;
    HardwareSerial *ports[3];
};

class ReadEvalPrintLoop : public ReadEvalPrintEvent, ReadEvalPrintProvider
{
public:
    ReadEvalPrintLoop();
    void run(HardwareSerial &serialPort, const ReadEvalPrintCommand *commands=NULL, uint32_t num_commands=0, ReadEvalPrintEventHandler handler=NULL, const char* help_header=NULL);

    bool addProvider(ReadEvalPrintProvider &provider);
    void setGreeting(const char *greeting);
    void setPrompt(const char *prompt);

    virtual int commandIndex();
    virtual int numArguments();
    virtual const char * getArgument(int arg);
    virtual bool invalidParameter(int arg, const char* msg="");
    virtual bool tolong(int arg, int *value);

    const ReadEvalPrintCommand* getTable(uint32_t *num_commands);
    bool event(ReadEvalPrintEvent &event, Print &port);
    const char* getHelpHeader() {return "REPL Settings";}

protected:
    HardwareSerial *port;
    char buffer[256];
    char *argv[READEVALPRINT_MAX_ARGS];
    uint32_t argc;

    ReadEvalPrintProvider *providers[READEVALPRINT_MAX_PROVIDERS];
    uint32_t provider_count;
    const ReadEvalPrintCommand *commands;
    int cmd;
    bool quit;
    bool echo;
    bool prompting;
    const char *prompt;
    const char *greeting;

    bool eval();
    int match(ReadEvalPrintProvider &p);
    void printUsage(uint32_t row);
    void printHelp(ReadEvalPrintProvider &p, bool advanced);
    void printEcho();
    void printPrompting();
    virtual void sleep(){}
    virtual void worker(){}
};

extern ReadEvalPrintLoop ReadEvalPrint;
