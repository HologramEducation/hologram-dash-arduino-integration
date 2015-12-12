/*
  sst26vf016b_id.ino

  http://konekt.io

  Copyright (c) 2015 Konekt, Inc.  All rights reserved.

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


//Uncomment any of the below to use alternate SPI pins
//#define SPI_SCK SPI0_SCK_D08
//#define SPI_SIN SPI0_SIN_D27
//#define SPI_SOUT SPI0_SOUT_D26

//Uncomment to use an alternate default CS pin, or pass the alternate pin to the
//beginTransaction function, e.g.
//SPI.beginTransaction(SPI0_CS_D05);

//#define SPI_CS SPI0_CS_U11
//#define SPI_CS SPI0_CS_D05


#include <SPI.h>

//SPI Example that prints the 3-byte ID register of the SST26VF016B SPI Flash.
//Setup:
//   SST  <-->       Dash/DashPro  DashProBeta
// 1 CE#  <--> CS    D02           U16
// 2 SO   <--> SOUT  D12           U14
// 3 WP#  <--> 3.3V
// 4 Vss  <--> GND
// 5 SI   <--> SIN   D14           U15
// 6 SCK  <--> SCK   D11           U13
// 7 HOLD <--> 3.3V
// 8 Vdd  <--> 3.3V

void setup() {
  // put your setup code here, to run once:
  Serial0.begin(115200);
  SPI.begin();
  delay(1000);
  Serial0.println("SPI Example");
}

uint32_t count = 0;
uint8_t id[3];
void loop() {
  // put your main code here, to run repeatedly:
  memset(id, 0xEE, 3);
  Serial0.print("Loop "); Serial0.println(count++);
  SPI.beginTransaction(SPI_CS); //enables the specified chip select output
  SPI.transfer(0x9F);           //requests the ID register
  SPI.transfer(id, 3);          //read the 3 bytes of the ID register
  SPI.endTransaction();         //release the chip select
  Serial0.print("ID: 0x");      //print the result. ID for SST26VF016B is 0xBF2641
  Serial0.print(id[0], HEX);
  Serial0.print(id[1], HEX);
  Serial0.println(id[2], HEX);
  delay(500);
}
