/*
MCUFlash.cpp - Class definitions that provide a Flash subclass for the internal
flash memory for a Kinetis MCU.

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
#include "MCUFlash.h"
#include "wiring_digital.h"

#define USER_FLASH_OFFSET (786432U)
#define USER_FLASH_SIZE (262144U)

static const FLASH_SSD_CONFIG flashconfig = {
    .ftfxRegBase = 1073872896U,
    .PFlashBase  = 0U,
    .PFlashSize  = 1048576U,
    .DFlashBase  = 0U,
    .DFlashSize  = 0x00U,
    .EERAMBase   = 0U,
    .EEESize     = 0x00U,
    .DebugEnable = false,
    .CallBack    = NULL_CALLBACK,
}; 

MCUFlash::MCUFlash()
:Flash(4096, 256), readPtr(0), writeCount(0), writeAddress(0){}

void MCUFlash::begin() {
    g_FlashLaunchCommand = (pFLASHCOMMANDSEQUENCE)RelocateFunction((uint32_t)ramFunc, 50, (uint32_t)FlashCommandSequence);
    begun = true;
}

uint32_t MCUFlash::read(uint32_t address, uint8_t *buffer, size_t count) {
    if(!ready()) return 0;
    if(address > USER_FLASH_SIZE) return 0;
    if(count + address > USER_FLASH_SIZE) count = USER_FLASH_SIZE - address;
    memcpy(buffer, (void*)(address+USER_FLASH_OFFSET), count);
    return count;
}

uint32_t MCUFlash::write(uint32_t address, const void *buffer, size_t count) {
    if(!ready()) return 0;
    if(address > USER_FLASH_SIZE) return 0;
    if((address & 0x7) != 0) return 0;
    if(count + address > USER_FLASH_SIZE) count = USER_FLASH_SIZE - address;
    __disable_irq();
    uint32_t result = FlashProgram(&flashconfig, address+USER_FLASH_OFFSET, count, (uint8_t*)buffer, g_FlashLaunchCommand);
    __enable_irq();
    return result == FTFx_OK ? count : 0;
}

bool MCUFlash::eraseSector(uint32_t address) {
    if(!ready()) return false;
    if(address > USER_FLASH_SIZE) return false;
    __disable_irq();
    uint32_t result = FlashEraseSector(&flashconfig, address+USER_FLASH_OFFSET, FTFx_PSECTOR_SIZE, g_FlashLaunchCommand);
    __enable_irq();
    return result == FTFx_OK;
}

bool MCUFlash::eraseAll()
{
    if(!ready()) return false;
    for(uint32_t i=0; i<USER_FLASH_SIZE; i+=sectorSize) {
        eraseSector(i);
    }
    return true;
}

bool MCUFlash::beginRead(uint32_t address)
{
    if(!ready()) return false;
    if(address > USER_FLASH_SIZE) return false;
    readPtr = (uint8_t*)(address + USER_FLASH_OFFSET);
}

uint8_t MCUFlash::continueRead()
{
    if(readPtr >= (uint8_t*)(USER_FLASH_SIZE + USER_FLASH_OFFSET)) return 0xFF;
    return *readPtr++;
}

bool MCUFlash::beginWrite(uint32_t address)
{
    if(!ready()) return false;
    if((address & 0x7) != 0) return false;
    if(address > USER_FLASH_SIZE) return false;
    writeCount = 0;
    writeAddress = address;
    return true;
}

bool MCUFlash::continueWrite(uint8_t byte)
{
    bool result = true;
    if(writeCount == PGM_SIZE_BYTE) {
        result = endWrite();
        writeAddress += PGM_SIZE_BYTE;
    }
    writeBuffer[writeCount++] = byte;
    return result;
}

bool MCUFlash::endWrite()
{
    uint32_t written = 0;
    if(writeCount)
    {
        written = write(writeAddress, writeBuffer, writeCount);
        writeCount = 0;
    }
    return written == writeCount;
}
