/*
  ReadEvalPrint.cpp - processes commands in a Read-Eval-Print-Loop.

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
#include "ReadEvalPrint.h"

ReadEvalPrintBaseProvider::ReadEvalPrintBaseProvider(const ReadEvalPrintCommand *commands, uint32_t num_commands, ReadEvalPrintEventHandler handler, const char* help_header)
:commands(commands), command_count(num_commands), handler(handler), header(help_header)
{
    if(header == NULL)
        header = "";
}

const ReadEvalPrintCommand* ReadEvalPrintBaseProvider::getTable(uint32_t *num_commands)
{
    *num_commands = this->command_count;
    return this->commands;
}

bool ReadEvalPrintBaseProvider::event(ReadEvalPrintEvent &event, Print &port)
{
    if(handler) {
        return handler(event, port);
    }
    return false;
}

SerialMirror::SerialMirror()
: portcount(0)
{}

bool SerialMirror::add(HardwareSerial &serialPort) {
    if(portcount >= 3) return false;
    ports[portcount++] = &serialPort;
}

void SerialMirror::begin(unsigned long baudrate) {
    for(int i=0; i<portcount; i++)
        ports[i]->begin(baudrate);
}

void SerialMirror::begin(unsigned long baudrate, uint16_t config) {
    for(int i=0; i<portcount; i++)
        ports[i]->begin(baudrate, config);
}

void SerialMirror::end() {
    for(int i=0; i<portcount; i++)
        ports[i]->end();
}

int SerialMirror::available() {
    int max = 0;
    for(int i=0; i<portcount; i++) {
        int a = ports[i]->available();
        if(a > max)
            max = a;
    }
    return max;
}

int SerialMirror::peek() {
    for(int i=0; i<portcount; i++) {
        int p = ports[i]->peek();
        if(p != -1)
            return p;
    }
    return -1;
}

int SerialMirror::read() {
    for(int i=0; i<portcount; i++) {
        int r = ports[i]->read();
        if(r != -1)
            return r;
    }
    return -1;
}

void SerialMirror::flush() {
    for(int i=0; i<portcount; i++)
        ports[i]->flush();
}

size_t SerialMirror::write(uint8_t data) {
    for(int i=0; i<portcount; i++)
        ports[i]->write(data);
    return 1;
}

const char *GREETING = "type 'help' for commands, 'quit' to exit";
const char *PROMPT = "> ";

ReadEvalPrintLoop::ReadEvalPrintLoop()
: echo(true), quit(false), greeting(GREETING), prompt(PROMPT),
prompting(true)
{}

void ReadEvalPrintLoop::setGreeting(const char *greeting)
{
    this->greeting = greeting;
}

void ReadEvalPrintLoop::setPrompt(const char *prompt)
{
    this->prompt = prompt;
}

static const ReadEvalPrintCommand TABLE[] = {
    {1, 1, "print this help text",                  "help", "[all]"},
    {1, 0, "quit the repl",                         "quit"},
    {1, 0, "print echo status",                     "echo"},
    {2, 0, "turn on echo",                          "echo", "on"},
    {2, 0, "turn off echo",                         "echo", "off"},
    {1, 9, "print the message, in quotes",          "print", "[message]"},
    {1, 0, "print the prompt status",               "prompt"},
    {2, 0, "turn the prompt on",                    "prompt", "on"},
    {2, 0, "turn the prompt off",                   "prompt", "off"},
};

const ReadEvalPrintCommand* ReadEvalPrintLoop::getTable(uint32_t *num_commands)
{
    *num_commands = sizeof(TABLE)/sizeof(ReadEvalPrintCommand);
    return TABLE;
}

bool ReadEvalPrintLoop::event(ReadEvalPrintEvent &event, Print &port)
{
    switch(event.commandIndex()) {
    case 0: { //help
            bool adv = false;
            if(event.numArguments() == 2)
                adv = (strcmp(event.getArgument(1), "all") == 0);
            printHelp(*this, adv);
            for(int i=0; i<provider_count; i++) {
                printHelp(*providers[i], adv);
            }
        }
        break;
    case 1: quit = true; break;
    case 2: printEcho(); break;
    case 3: echo = true; printEcho(); break;
    case 4: echo = false; printEcho(); break;
    case 5:
        for(int i=1; i<event.numArguments(); i++) {
            if(i>1) port.write(' ');
            port.print(event.getArgument(i));
        }
        port.println();
        break;
    case 6: printPrompting(); break;
    case 7: prompting = true; printPrompting(); break;
    case 8: prompting = false; printPrompting(); break;
    default:
        return false;
    }
    return true;
}

void ReadEvalPrintLoop::printEcho()
{
    port->print("echo ");
    port->println(echo ? "on" : "off");
}

void ReadEvalPrintLoop::printPrompting()
{
    port->print("prompt ");
    port->println(prompting ? "on" : "off");
}

int ReadEvalPrintLoop::commandIndex()
{
    return cmd;
}

int ReadEvalPrintLoop::numArguments()
{
    return argc;
}

const char * ReadEvalPrintLoop::getArgument(int arg)
{
    if(arg < argc)
        return argv[arg];
    return NULL;
}

bool ReadEvalPrintLoop::invalidParameter(int arg, const char* msg)
{
    int offset = (int)(argv[arg] - buffer);
    for(int i=0; i<offset+strlen(prompt); i++)
        port->write('-');
    port->println("^");
    port->print("Invalid value ");
    port->println(msg);
    port->println("Usage:");
    printUsage(cmd);
    return true;
}

bool ReadEvalPrintLoop::tolong(int arg, int *value)
{
    char *pEnd;
    *value = strtol(argv[arg], &pEnd, 0);
    if((pEnd[0] != 0) || (*value == 0 && (argv[arg][0] != '0' || argv[arg][1] != 0))) {
        return false;
    }
    return true;
}

int ReadEvalPrintLoop::match(ReadEvalPrintProvider &p)
{
    bool matched = true;
    uint32_t count = 0;
    commands = p.getTable(&count);

    for(int row=0; row<count; row++) {                                          //iterate through each row in the table
        if(argc < commands[row].req || argc > (commands[row].req+commands[row].opt)) {      //if argc not in range
            continue;                                                           //go to the next row
        }

        matched = true;

        for(int col=0; col<commands[row].req; col++) {                              //iterate through each column in the row
            if(commands[row].argv[col][0] == 0) continue;
            if(strcmp(commands[row].argv[col], argv[col]) != 0) {                   //if it doesn't match the arg, try the next row
                if(commands[row].argv[col][0] != '<') {
                    matched = false;
                    break;
                }
            }
        }

        if(matched) {                                                           //all the columns matched all the
            return row;
        }
    }

    return -1;
}

void ReadEvalPrintLoop::printUsage(uint32_t row)
{
    uint32_t sum = 0;
    for(int param=0; param<READEVALPRINT_MAX_ARGS; param++) {
        uint32_t len = strlen(commands[row].argv[param]);
        if(len) {
            sum += len+1;
            port->print(commands[row].argv[param]);
            port->write(' ');
        } else {
            if(commands[row].req + commands[row].opt > param) {
                port->print("... ");
                sum += 4;
            }
            break;
        }
    }

    for(int j=sum; j<40; j++) {
        port->write(' ');
    }
    port->println(commands[row].help);
}

void ReadEvalPrintLoop::printHelp(ReadEvalPrintProvider &p, bool advanced)
{
    uint32_t count = 0;
    commands = p.getTable(&count);

    if(!p.advanced() || advanced) {
        port->println();
        if(strlen(p.getHelpHeader()))
            port->println(p.getHelpHeader());
        port->print("----------------------------------------");
        port->println("----------------------------------------");

        for(int row=0; row<count; row++) {
            printUsage(row);
        }
    }
}

typedef enum
{
    EVAL_NEXT_TOKEN,
    EVAL_IN_TOKEN,
    EVAL_IN_QUOTE,
    EVAL_IN_ESCAPE,
    EVAL_COMPLETE,
}eval_state;

bool ReadEvalPrintLoop::eval()
{
    eval_state state = EVAL_NEXT_TOKEN;
    uint32_t idx = 0;
    argc = 0;
    argv[0] = buffer;
    bool getNewline = false;
    bool error = false;

    while(state != EVAL_COMPLETE) {
        worker();
        if(!port->available()) {
            sleep();
            continue;
        }

        char c = (char)port->read();

        if(echo & (c != '\r' && c != '\n')) port->write(c);

        switch(state) {
            case EVAL_NEXT_TOKEN: {
                switch(c) {
                    case '\r':
                        getNewline = true;
                    case '\n':
                        state = EVAL_COMPLETE;
                        break;
                    case '"':
                        state = EVAL_IN_QUOTE;
                        argv[argc++] = &buffer[idx];
                        break;
                    default:
                        state = EVAL_IN_TOKEN;
                        argv[argc++] = &buffer[idx];
                    case ' ':
                        buffer[idx++] = c;
                        break;
                }
                break;
            }
            case EVAL_IN_TOKEN: {
                switch(c) {
                    case '\r':
                        getNewline = true;
                    case '\n':
                        state = EVAL_COMPLETE;
                        break;
                    case ' ':
                        state = EVAL_NEXT_TOKEN;
                        buffer[idx++] = 0;
                        break;
                    default:
                        buffer[idx++] = c;
                        break;
                }
                break;
            }
            case EVAL_IN_QUOTE: {
                switch(c) {
                    case '\r':
                    case '\n':
                        error = true;
                        state = EVAL_COMPLETE;
                        break;
                    case '\\':
                        state = EVAL_IN_ESCAPE;
                        break;
                    case '"':
                        state = EVAL_NEXT_TOKEN;
                        buffer[idx++] = 0;
                        break;
                    default:
                        buffer[idx++] = c;
                        break;
                }
                break;
            }
            case EVAL_IN_ESCAPE: {
                switch(c) {
                    case '\r':
                    case '\n':
                        error = true;
                        state = EVAL_COMPLETE;
                        break;
                    default:
                        buffer[idx++] = '\\';
                    case '"':
                        state = EVAL_IN_QUOTE;
                        buffer[idx++] = c;
                        break;
                }
                break;
            }
        }
    }

    if(getNewline) {
        delay(3);
        if(port->available()) {
            if(port->peek() == '\n')
                port->read();
        }
    }

    buffer[idx] = 0;

    if(echo) port->println();

    if(error) {
        port->println("Improperly formatted command");
        return true;
    }

    if(argc == 0) {
        return true;
    }

    bool valid = false;

    cmd = match(*this);
    if(cmd >= 0)
    {
        valid = event(*this, *port);
        if(valid)
            return !quit;
    } else {
        for(int i=0; i<provider_count; i++) {
            cmd = match(*providers[i]);
            if(cmd >= 0) {
                valid = providers[i]->event(*this, *port);
                break;
            }
        }
    }

    if(!valid)
        port->println("Unknown command");

    return true;
}

bool ReadEvalPrintLoop::addProvider(ReadEvalPrintProvider &provider)
{
    if(provider_count < READEVALPRINT_MAX_PROVIDERS) {
        providers[provider_count++] = &provider;
        return true;
    }
    return false;
}

void ReadEvalPrintLoop::run(HardwareSerial &serialPort, const ReadEvalPrintCommand *commands, uint32_t num_commands, ReadEvalPrintEventHandler handler, const char* help_header)
{
    quit = false;

    port = &serialPort;
    ReadEvalPrintBaseProvider p(commands, num_commands, handler, help_header);
    if(commands && num_commands > 0 && handler) {
        addProvider(p);
    }
    port->println();
    port->println(greeting);
    port->println();
    bool again = true;
    while(again) {
        if(prompting) port->print(prompt);
        again = eval();
    }
    if(commands && num_commands > 0 && handler)
        provider_count--;
}

ReadEvalPrintLoop ReadEvalPrint;
