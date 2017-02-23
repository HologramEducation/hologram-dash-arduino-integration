/*
  repl_provider.ino - processes custom commands in a Read-Eval-Print-Loop.

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

#include <ReadEvalPrint.h>
#include "CustomProvider.h"

CustomProvider custom;

void setup() {
  Serial.begin();
  ReadEvalPrint.setGreeting("Welcom to the Terminal!");
  ReadEvalPrint.setPrompt("REPL> ");
  ReadEvalPrint.addProvider(custom);
}

void loop() {
  //open the Serial Monitor and type help for a list of commands
  ReadEvalPrint.run(Serial);
}
