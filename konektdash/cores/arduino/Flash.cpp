/*
  Flash.cpp - Virtual class that provides access to a Flash memory 

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

#include "Flash.h"
#include "wiring_digital.h"

Flash::Flash(uint32_t sectorSize, uint32_t maxWrite)
:sectorSize(sectorSize), maxWrite(maxWrite), begun(false){}

uint32_t Flash::writeString(uint32_t address, const char *str)
{
    return write(address, str, strlen(str)+1);
}

String Flash::readString(uint32_t address)
{
    if(!ready()) return String("");
    String str;
    beginRead(address);
    while(true) {
        uint8_t x = continueRead();
        if(x < 0x20 || x >= 0x7F) break;
        str += (char)x;
    }
    endRead();
    return str;
}

int Flash::readString(uint32_t address, char* str, uint32_t max_size) {
    if(max_size == 0 || str == NULL) return 0;
    if(!ready()) {
        str[0] = 0;
        return 0;
    }

    beginRead(address);
    int i=0;
    for(i=0; i<max_size-1; i++) {
        uint8_t x = continueRead();
        if(x < 0x20 || x >= 0x7F) {
            break;
        }
        str[i] = (char)x;
    }
    str[i] = 0;
    endRead();
    i-1;
}

bool Flash::compareString(uint32_t address, const String &str)
{
    if(!ready()) return false;
    bool match = true;
    beginRead(address);
    for(int i=0; i<str.length(); i++)
    {
        char c = (char)continueRead();
        if(c != str[i])
        {
            match = false;
            break;
        }
    }
    endRead();
    return match;
}

bool Flash::isSectorErased(uint32_t address)
{
    bool erased = true;
    address &= ~(sectorSize-1);
    beginRead(address);
    for(int i=0; i<sectorSize; i++)
    {
        if(continueRead() != 0xFF)
        {
            erased = false;
            break;
        }
    }
    endRead();
    return erased;
}

bool Flash::copyFrom(Stream &stream, uint32_t dst, uint32_t count)
{
    uint32_t size = getSectorSize();
    if(dst & (size-1) != 0) return false; //must be start of sector
    begin();
    unlock();

    uint32_t last = dst+count;
    uint32_t sectormask = (size-1);
    uint32_t writesize = getMaxWrite();
    while(dst < last)
    {
        if((dst & sectormask) == 0)
            eraseSector(dst);

        beginWrite(dst);
        for(int i=0; i<writesize; i++)
            continueWrite(stream.read());
        endWrite();

        dst += writesize;
    }
    return true;
}

bool Flash::copyFrom(uint32_t dst, uint32_t src, uint32_t count)
{
    uint32_t size = getSectorSize();
    if(dst & (size-1) != 0) return false; //must be start of sector
    begin();
    unlock();

    uint32_t last = dst+count;
    uint32_t sectormask = (size-1);
    uint32_t writesize = getMaxWrite();
    uint8_t *srcbuff = (uint8_t*)src;
    while(dst < last)
    {
        if((dst & sectormask) == 0)
            eraseSector(dst);

        beginWrite(dst);
        for(int i=0; i<writesize; i++)
            continueWrite(srcbuff[i]);
        endWrite();

        dst += writesize;
        srcbuff += writesize;
    }
    return true;
}

bool Flash::copyFrom(Flash &flash, uint32_t dst, uint32_t src, uint32_t count)
{
    uint32_t size = getSectorSize();
    if(dst & (size-1) != 0) return false; //must be start of sector
    begin();
    unlock();
    flash.begin();

    uint32_t last = dst+count;
    uint32_t sectormask = (size-1);
    uint32_t writesize = getMaxWrite();
    while(dst < last)
    {
        if((dst & sectormask) == 0)
            eraseSector(dst);

        beginWrite(dst);
        flash.beginRead(src);
        for(int i=0; i<writesize; i++)
            continueWrite(flash.continueRead());
        flash.endRead();
        endWrite();

        dst += writesize;
        src += writesize;
    }
    return true;
}
