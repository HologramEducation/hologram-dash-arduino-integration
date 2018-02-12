/*
  hologram_dash_cloud.ino - demonstrate features of the HologramCloud
  This sketch will send a message to the cloud every 30 minutes.
  It will also send a cloud message on receive of an SMS.

  https://hologram.io

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
  HologramCloud.attachTopic("rx_sms");
  HologramCloud.sendMessage();
}

#define SIZE_INBOUND 4096               //Inbound message size limit
char buffer_inbound[SIZE_INBOUND];      //Holds inbound message

//Process a received inbound message
void cloud_inbound(int length) {
  buffer_inbound[length] = 0; //NULL terminate the data for printing as a String

  HologramCloud.sendMessage(buffer_inbound, "inbound");
}

void cloud_notify(cloud_event e) {
  switch(e) {
    case CLOUD_EVENT_CONNECTED:
      //re-open the server socket on port 4010
      HologramCloud.listen(4010);
      break;
  }
}

void setup() {
  //On startup the modem attempts to connect to the cell network, if available

  //Use the cloud_sms function to process incoming SMS
  HologramCloud.attachHandlerSMS(cloud_sms);

  //Use the cloud_inbound function to process a received message.
  //Uses a buffer and size defined in the sketch for user requirements.
  //If String data, reserve one byte for NULL-terminator
  //The full buffer size can be used for binary or non-printed data.
  HologramCloud.attachHandlerInbound(cloud_inbound,
                                     buffer_inbound,
                                     SIZE_INBOUND-1);

  HologramCloud.attachHandlerNotify(cloud_notify);

  while(!HologramCloud.isConnected()) {
    Dash.snooze(1000);
  }

  //sync clock with network time
  rtc_datetime_t dt;
  if(HologramCloud.getNetworkTime(dt)) {
    Clock.setDateTime(dt);
  }

  //Send a message to the Hologram Cloud with the topic "cloud_demo"
  //Will connect to the Hologram Cloud automatically
  HologramCloud.sendMessage("Hologram Cloud Demo Active", "cloud_demo");
}

void loop() {
  //Blink the LED while the Dash is awake
  Dash.pulseLED(200, 200);

  //Set the Clock Alarm to wake the Dash up
  Clock.setAlarmMinutesFromNow(5);

  while(!HologramCloud.isConnected()) {
    Dash.snooze(1000);
  }

  //Open a server socket on port 4010 to receive messages
  HologramCloud.listen(4010);

  //Buffer a message to send to the Hologram Cloud
  HologramCloud.print("A01: ");
  HologramCloud.println(analogRead(A01));
  HologramCloud.print("Battery: ");
  HologramCloud.print(FuelGauge.percentage());
  HologramCloud.println("%");
  HologramCloud.print("Signal Strength: ");
  HologramCloud.println(HologramCloud.getSignalStrength());
  //Attach topics to the message for advanced routing and processing
  HologramCloud.attachTopic("A01");
  HologramCloud.attachTopic("Battery");
  HologramCloud.attachTopic("SignalStrength");

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
      case CLOUD_ERR_UNREGISTERED:
        //Low signal, check the antenna, or move into better coverage area
        Dash.snooze(10000); //wait 10 seconds

        //The last message is buffered and can be re-sent again (on success
        //or failure).
        //Attempt to send the buffered message again.
        HologramCloud.sendMessage();

        //Any call to write to the buffer or attach a topic or
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
  while(!Clock.alarmExpired()) {
    HologramCloud.pollEvents();
    Dash.sleep();
  }

  //The LED is turned off in deep sleep, so turn it on now
  Dash.onLED();

  //Wake the Dash again in 25 minutes
  Clock.setAlarmMinutesFromNow(25);

  //Manually check for events, such as an SMS received while in deep sleep
  //Optional here because events are checked automatically before powering
  //down the modem in the next line...
  HologramCloud.pollEvents();

  //Done with the modem for now, turn it off to save power
  //This closes all sockets
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
