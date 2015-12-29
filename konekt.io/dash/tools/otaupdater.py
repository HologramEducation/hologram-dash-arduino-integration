#
#  otaupdater.py - Application to push firmware images to a board via
#  Konekt OTA Updater API endpoints
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

import easygui
import requests
import argparse
import os.path
import sys
import json

class UpdaterException(Exception):
    pass

class OTAUpdaterGUI:
    def prompt_for_apikey(self):
        return easygui.passwordbox(
                msg="Please paste in your Konekt API key",
                title="Konekt OTA Updater")

    def prompt_for_device(self, devices):
        device_list = []
        device_map = {}
        for device in devices:
            device_list.append(device['name'])
            device_map[device['name']] = device['id']
        res = easygui.choicebox(
                msg='Choose the device to update',
                title='Konekt OTA Updater',
                choices=device_list)
        if res == None:
            return None
        return device_map[res]

class KonektOTAUpdater:
    apibase = 'https://dashboard.konekt.io/api/1/'
    def __init__(self, imagefile):
        self.apikey = None
        self.deviceid = None
        self.imagefile = imagefile
        self.ui = OTAUpdaterGUI()

    def set_apikey(self, apikey):
        self.apikey = apikey

    def set_deviceid(self, deviceid):
        self.deviceid = deviceid

    def set_apibase(self, apibase):
        self.apibase = apibase

    def load_userinfo(self):
        apiurl = self.apibase + 'users/me/'
        url_params = {'apikey' : self.apikey}
        r = requests.get(apiurl, params=url_params)
        if r.status_code != requests.codes.ok:
            raise UpdaterException('Error connecting to API: ' + r.text)
        else:
            resp = r.json();
            return resp['data']

    def load_devices(self, userid):
        apiurl = self.apibase + 'devices/'
        url_params = {'apikey' : self.apikey,
                'userid' : userid}
        r = requests.get(apiurl, params=url_params)
        if r.status_code != requests.codes.ok:
            raise UpdaterException('Error connecting to API: ' + r.text)
        else:
            resp = r.json();
            return resp['data']

    def push_update(self):
        if(not os.path.exists(self.imagefile) or
                not os.path.isfile(self.imagefile)):
            raise IOError('Image file %s does not exist' % self.imagefile)
        if not self.apikey:
            self.apikey = self.ui.prompt_for_apikey()
            if not self.apikey:
                raise UpdaterException('No API key entered')
        if not self.deviceid:
            userinfo = self.load_userinfo()
            devices = self.load_devices(userinfo['id'])
            self.deviceid = self.ui.prompt_for_device(devices)
            if not self.deviceid:
                raise UpdaterException('No device ID entered')
        print("Pushing firmware to device ID #%d" % self.deviceid)
        #upload image
        apiurl = self.apibase + 'firmwareimages/'
        url_params = {'apikey' : self.apikey}
        fname = os.path.basename(self.imagefile)
        files = {'imagefile': (fname, open(self.imagefile, 'rb'),
            'application/octet-stream')}
        print("Uploading")
        r = requests.post(apiurl, files=files, params=url_params)
        if r.status_code != requests.codes.ok:
            raise UpdaterException('Error uploading image: ' + r.text)
        resp = r.json();
        imageobj = resp['data']
        fwid = imageobj['id']
        print("Firmware ID #%d created" % fwid)
        print("Executing push to device")
        sendurl = self.apibase + 'firmwareimages/' + str(fwid) + '/send'
        payload = { 'deviceids': [self.deviceid] }
        headers = {'Content-Type':'application/json'}
        r = requests.post(sendurl,
                params=url_params,
                data=json.dumps(payload),
                headers=headers)
        if r.status_code != requests.codes.ok:
            raise UpdaterException('Error pushing image: ' + r.text)
        print("OTA update sent")


def main():
    requests.packages.urllib3.disable_warnings()

    parser = argparse.ArgumentParser(description='Push a firmware image to a device',
            add_help=True)
    parser.add_argument('--imagefile', type=str, required=True)
    parser.add_argument('--apikey', type=str)
    parser.add_argument('--deviceid', type=int)
    parser.add_argument('--apibase', type=str, help=argparse.SUPPRESS)
    args = parser.parse_args()
    updater = KonektOTAUpdater(args.imagefile)
    if args.apikey:
        updater.set_apikey(args.apikey)
    if args.deviceid:
        updater.set_deviceid(args.deviceid)
    if args.apibase:
        updater.set_apibase(args.apibase)
    try:
        updater.push_update()
    except UpdaterException as e:
        print('Error! ' + str(e))
        sys.exit(0)
    except IOError as e:
        print('File Error! ' + str(e))
        sys.exit(0)


if __name__ == "__main__":
    main()

