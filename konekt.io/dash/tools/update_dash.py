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

g_verbose=True
g_log_file = "konekt_loader_log.txt"

def log(msg,fh=None):
    verbose(msg)
    if (fh != None):
        fh.write(msg)
        fh.write("\n")
    
def verbose(msg):
    if g_verbose:
        print msg
    
def update(in_file, block_id=0x18, reset_id=0xFC, vid=0x7722, pid=0x1200, log_file_handler=None):
    with open(in_file, 'rb') as f:
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
                    log("block write failed", log_file_handler)
                    break
                num += 1
            block = [0]*1089
            block[1] = reset_id
            block[4] = reset_id
            n = h.write(block)
            if n != 1089:
                log("erase " + hex(reset_id) + " failed", log_file_handler)
        finally:
            h.close()
    
if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Konekt DashPro Loader')
    parser.add_argument('path', help='path to DashPro binary image')
    args = parser.parse_args()
    f = None
    try:
        f = open(g_log_file,'w')
        verbose("Output being logged to: " + g_log_file)
    except:
        f = None
    update(args.path, log_file_handler=f)
    if (f != None):
        f.close()
        verbose("Log file written to: " + g_log_file)
