/*
  sst26vf016b_id.ino

  http://konekt.io

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

//Any available digital IO pin can be used as the Chip Select
#define SPI_CS L07

#include <SPI.h>

//SPI Example that prints the 3-byte ID register of the SST26VF016B SPI Flash.
//Setup:
//   SST  <-->       Dash/DashPro  DashProBeta
// 1 CE#  <--> CS    L07           U17
// 2 SO   <--> SIN   L08           U14
// 3 WP#  <--> 3.3V
// 4 Vss  <--> GND
// 5 SI   <--> SOUT  L06           U15
// 6 SCK  <--> SCK   L05           U13
// 7 HOLD <--> 3.3V
// 8 Vdd  <--> 3.3V

//AltSPI Setup, using alternate SOUT and SIN pins (Dash Only)
// To use, replace SPI with AltSPI
//   SST  <-->       Dash
// 1 CE#  <--> CS    L07
// 2 SO   <--> SIN   R14
// 3 WP#  <--> 3.3V
// 4 Vss  <--> GND
// 5 SI   <--> SOUT  R13
// 6 SCK  <--> SCK   L05
// 7 HOLD <--> 3.3V
// 8 Vdd  <--> 3.3V

void setup() {
  // put your setup code here, to run once:
  Serial.begin();
  SPI.begin();
  delay(1000);
  Serial.println("SPI Example");
}

uint32_t count = 0;
uint8_t id[3];
void loop() {
  // put your main code here, to run repeatedly:
  memset(id, 0xEE, 3);
  Serial.print("Loop "); Serial.println(count++);
  SPI.beginTransaction(SPI_CS); //enables the specified chip select output
  SPI.transfer(0x9F);           //requests the ID register
  SPI.transfer(id, 3);          //read the 3 bytes of the ID register
  SPI.endTransaction();         //release the chip select
  Serial.print("ID: 0x");      //print the result. ID for SST26VF016B is 0xBF2641
  Serial.print(id[0], HEX);
  Serial.print(id[1], HEX);
  Serial.println(id[2], HEX);
  delay(500);
}
