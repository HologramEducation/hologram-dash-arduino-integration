#
#  update_dash.py - Loader application to load firmware .bin images
#  to the Konekt Dash and Konekt Dash Pro boards and boards with
#  compatible bootloaders.  Can be used with Arduino IDE Integration
#  or as a standalone loader application. Requires: cython, cython-hid
#  
#  http://konekt.io
#  
#  Copyright (c) 2015 Konekt, Inc.  All rights reserved.
#  
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 2.1 of the License, or (at your option) any later version.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#

import hid
import sys
import argparse

def update(file, block_id=0x18, reset_id=0xFC, vid=0x7722, pid=0x1200):
    with open(file, 'rb') as f:
        h = hid.device()
        try:
            h.open(vid, pid)
            num = 0
            while(f):
                kb = f.read(1024)
                if(len(kb) == 0):
                    break
                block = [0]*65 + [x for x in bytearray(kb)]
                block[1] = block_id
                block[4] = block_id
                block[5] = num & 0xFF
                block[6] = (num >> 8) & 0xFF
                n = h.write(block)
                if n != 1089:
                    print "block write failed"
                    break
                num += 1
            block = [0]*1089
            block[1] = reset_id
            block[4] = reset_id
            n = h.write(block)
            if n != 1089:
                print "erase", hex(reset_id), "failed"
        finally:
            h.close()
    
if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Konekt DashPro Loader')
    parser.add_argument('path', help='path to DashPro binary image')
    args = parser.parse_args()
    update(args.path)
    
