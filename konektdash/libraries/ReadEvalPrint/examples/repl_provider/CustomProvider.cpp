/*
  CustomProvider.cpp - custom commands for a Read-Eval-Print-Loop.

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

#include "CustomProvider.h"

static const ReadEvalPrintCommand TABLE[] = {
    {1, 0, "print yo",                                  "yo"},
    {3, 7, "add up to 9 numbers",                       "add", "<augend>", "<addend>"},
    {3, 0, "subtract the subtrahend from the minuend",  "sub", "<minuend>", "<subtrahend>"},
};

const ReadEvalPrintCommand* CustomProvider::getTable(uint32_t *num_commands)
{
  *num_commands = sizeof(TABLE)/sizeof(ReadEvalPrintCommand);
  return TABLE;
}

bool CustomProvider::event(ReadEvalPrintEvent &event, Print &port)
{
  switch(event.commandIndex()) {
    case 0: //yo
      port.println("yo");
      break;
    case 1: //add
    {
      int sum=0, x=0;
      for(int i=1; i<event.numArguments(); i++) {
        if(event.tolong(i, &x)) {
          sum += x;
        } else {
          return event.invalidParameter(1);
        }
      }
      port.println(sum);
      break;
    }
    case 2: //sub
    {
      int a=0, b=0;
      if(event.tolong(1, &a)) {
        if(event.tolong(2, &b)) {
          port.println(a-b);
        } else {
          return event.invalidParameter(2);
        }
      } else {
        return event.invalidParameter(1);
      }
      break;
    }
    default: return false;
  }
  return true;
}
