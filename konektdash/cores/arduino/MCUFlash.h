/*
  MCUFlash.h - Class definitions that provide a Flash subclass for the internal
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
#pragma once

#include "Flash.h"
#ifdef __cplusplus
extern "C"{
#endif // __cplusplus
#include "SSD_FTFx.h"
#ifdef __cplusplus
}
#endif // __cplusplus

class MCUFlash : public Flash{
public:
    MCUFlash();

    virtual void begin();
    virtual uint32_t read(uint32_t address, uint8_t *buffer, size_t count);
    virtual uint32_t write(uint32_t address, const void *buffer, size_t count);
    virtual bool eraseSector(uint32_t address);
    virtual bool eraseAll();

    virtual bool beginRead(uint32_t address);
    virtual uint8_t continueRead() override;
    virtual bool beginWrite(uint32_t address);
    virtual bool continueWrite(uint8_t byte);
    virtual bool endWrite();

protected:
    uint8_t *readPtr;
    uint8_t writeBuffer[PGM_SIZE_BYTE];
    uint8_t writeCount;
    uint32_t writeAddress;

    pFLASHCOMMANDSEQUENCE g_FlashLaunchCommand;
    uint16_t ramFunc[25];
};
