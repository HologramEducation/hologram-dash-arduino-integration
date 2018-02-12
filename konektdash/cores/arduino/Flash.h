/*
  Flash.h - Virtual class that provides access to a Flash memory

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

#include "WString.h"
#include "Stream.h"

class Flash {
protected:
    uint32_t sectorSize;
    uint32_t maxWrite;
    bool begun;

public:
    Flash(uint32_t sectorSize, uint32_t maxWrite);

    uint32_t getSectorSize()    {return sectorSize;}
    uint32_t getMaxWrite()      {return maxWrite;}

    virtual bool ready()        {return begun;}
    virtual void begin()        {begun = true;}
    virtual void end()          {begun = false;}
    virtual void unlock()       {}

    bool compareString(uint32_t address, const String &str);
    bool compareString(uint32_t address, const char *str) { return compareString(address, String(str)); }

    String readString(uint32_t address);
    int readString(uint32_t address, char* str, uint32_t max_size);

    uint32_t writeString(uint32_t address, const String &str) { return writeString(address, str.c_str()); }
    uint32_t writeString(uint32_t address, const char *str);

    bool copyFrom(Stream &stream, uint32_t dst, uint32_t count);
    bool copyFrom(uint32_t dst, uint32_t src, uint32_t count);
    bool copyFrom(Flash &flash, uint32_t dst, uint32_t src, uint32_t count);

    bool isSectorErased(uint32_t address);

    virtual uint32_t read(uint32_t address, uint8_t *buffer, size_t count) = 0;
    virtual uint32_t write(uint32_t address, const void *buffer, size_t count) = 0;
    virtual bool eraseSector(uint32_t address) = 0;
    virtual bool eraseAll() = 0;

    virtual bool beginRead(uint32_t address) {}
    virtual uint8_t continueRead() = 0;
    virtual void endRead() {}
    virtual bool beginWrite(uint32_t address){}
    virtual bool continueWrite(uint8_t byte) = 0;
    virtual bool endWrite() {}
};
