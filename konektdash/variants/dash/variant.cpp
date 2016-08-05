/*
  variant.cpp

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

#include "variant.h"

void initVariant(void)
{
}

uint8_t USB_STR_PRODUCT[USB_STR_PRODUCT_SIZE + USB_STR_DESC_SIZE]
= { sizeof(USB_STR_PRODUCT),
    USB_STRING_DESCRIPTOR,
    'D', 0,
    'a', 0,
    's', 0,
    'h', 0,
    ' ', 0,
    'S', 0,
    'e', 0,
    'r', 0,
    'i', 0,
    'a', 0,
    'l', 0
};
