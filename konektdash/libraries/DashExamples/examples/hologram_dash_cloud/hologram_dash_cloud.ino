/*
  hologramcloud_demo.ino - demonstrate features of the HologramCloud
  This sketch will send a message to the cloud every 30 minutes.
  It will also send a cloud message on receive of an SMS.

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

//Process a received SMS
void cloud_sms(const String &sender, const rtc_datetime_t &timestamp, const String &message) {
  //Echo the received SMS back into the cloud
  HologramCloud.print("From: ");
  HologramCloud.print(sender);
  HologramCloud.print(" @ ");
  HologramCloud.print(timestamp);
  HologramCloud.print(" Message: ");
  HologramCloud.print(message);
  HologramCloud.attachTag("rx_sms");
  HologramCloud.sendMessage();
}

void setup() {
  //On startup the modem attempts to connect to the cell network, if available

  //Use the cloud_sms function to process incoming SMS
  HologramCloud.attachHandlerSMS(cloud_sms);

  //Send a message to the Hologram Cloud with the tag "cloud_demo"
  //Will connect to the Hologram Cloud automatically
  HologramCloud.sendMessage("Hologram Cloud Demo Active", "cloud_demo");

  //sync clock with network time
  rtc_datetime_t dt;
  if(HologramCloud.getNetworkTime(dt)) {
    Clock.setDateTime(dt);
  }
}

void loop() {
  //Blink the LED while the Dash is awake
  Dash.pulseLED(200, 200);

  //Set the Clock Alarm to wake the Dash up
  Clock.setAlarmMinutesFromNow(5);

  //Explicitly connect to the Hologram Cloud
  //This is optional because calling sendMessage() will connect as needed
  HologramCloud.connect();

  //Buffer a message to send to the Hologram Cloud
  HologramCloud.print("A01: ");
  HologramCloud.println(analogRead(A0));
  HologramCloud.print("Battery: ");
  HologramCloud.print(FuelGauge.percentage());
  HologramCloud.println("%");
  HologramCloud.print("Signal Strength: ");
  HologramCloud.println(HologramCloud.getSignalStrength());
  //Attach tags to the message for advanced routing and processing
  HologramCloud.attachTag("A01");
  HologramCloud.attachTag("Battery");
  HologramCloud.attachTag("SignalStrength");

  //Attempt to send the buffered message
  if(!HologramCloud.sendMessage()) {
    //if the message failed to send, try again
    int status = HologramCloud.getConnectionStatus();

    switch(status) {
      case CLOUD_ERR_SIM: //Is the SIM card inserted?
      case CLOUD_ERR_CONNECT: //Is the SIM card activated?
        //Can't easily recover from these errors, so give up on this message
        break;

      case CLOUD_ERR_SIGNAL:
        //Low signal, check the antenna, or move into better coverage area
        Dash.snooze(10000); //wait 10 seconds

        //The last message is buffered and can be re-sent again (on success
        //or failure).
        //Attempt to send the buffered message again.
        HologramCloud.sendMessage();

        //Any call to write to the buffer or attach a tag or
        //send a new message String will reset the buffer.
        HologramCloud.print("Test message");

        //Explicitly clear the message buffer. Usually not necessary unless
        //the buffer has been written but the contents should not be sent
        HologramCloud.clear();

        //"Test message" text is dropped, won't be sent
        break;
    }
  }

  //Keep the modem awake while the Dash is in DeepSleep to check for SMS
  if(!Clock.alarmExpired())
    Dash.deepSleep();

  //The LED is turned off in deep sleep, so turn it on now
  Dash.onLED();

  //Wake the Dash again in 25 minutes
  Clock.setAlarmMinutesFromNow(25);

  //Manually check for events, such as an SMS received while in deep sleep
  //Optional here because events are checked automatically before powering
  //down the modem in the next line...
  HologramCloud.pollEvents();

  //Done with the modem for now, turn it off to save power
  HologramCloud.powerDown();

  //only deep sleep if the alarm is still pending
  if(!Clock.alarmExpired())
    Dash.deepSleep();

  //Manually turn the modem back on
  //This is optional because the modem is automatically powered on when
  //commands using the cell network are called, such as signal strength
  //and sending messages.
  HologramCloud.powerUp();
}
//After each loop, if the modem is powered, modem events are checked.
