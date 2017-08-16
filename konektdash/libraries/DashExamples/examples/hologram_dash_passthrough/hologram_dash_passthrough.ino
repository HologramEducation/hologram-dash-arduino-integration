/*
  hologram_dash_passthrough.ino - put the Dash modem into passthrough mode.
  Use Serial to read/write directly with the modem.

  WARNING! This sketch allows writing AT commands directly to the modem. It is
  for advanced users only. This mode is provided as a convenience and is not
  officially supported by Hologram. Consult the modem AT command manual for
  supported commands.

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

void setup() {
  Serial.begin();

  //Requires System Firmware 0.9.10 or higher
  HologramCloud.enterPassthrough();
  //WARNING: Not officially supported. Advanced users only.
}

void loop() {
  while(Serial.available())
    SerialSystem.write(Serial.read());
  while(SerialSystem.available())
    Serial.write(SerialSystem.read());
}
