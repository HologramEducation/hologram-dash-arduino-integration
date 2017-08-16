/*
  Tone.h - Implements tone and related functions,
  with mods for the Konekt Dash and Konekt Dash Pro family

  https://hologam.io

  Copyright (c) 2017 Konekt, Inc.  All rights reserved.


  Derived from file with original copyright notice:

  Copyright (c) 2015 Arduino LLC.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#pragma once

#include "wiring_analog.h"

extern unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout=1000000);

void tone(uint8_t ulPin, unsigned int frequency, unsigned long duration = 0);
void noTone(uint8_t ulPin);

//returns true if a tone is currently being played
//can be used to block until the tone is finished
bool tonePlaying();

//callback function specified will be exectued once the tone duration is
//complete. Use for playing a song in the background with no active delay.
void attachToneInterrupt(void (*callback)(void));
