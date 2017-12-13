/*
  dash_repl_basic.ino - processes Dash commands in a Read-Eval-Print-Loop.

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

#include <DashReadEvalPrint.h>

SerialMirror Mirror;

//send a notifcation message on RTC alarm
//trigger the alarm to go off in 5 seconds with the command:
//alarm seconds 5
void alarm_handler(void)
{
  rtc_datetime_t dt;
  if(HologramCloud.getNetworkTime(dt))
    Clock.setDateTime(dt);
  HologramCloud.println("Alarm at ");
  HologramCloud.println(Clock.currentDateTime());
  HologramCloud.attachTag("alarm");
  if(HologramCloud.sendMessage())
    Mirror.println("Alarm notification sent");
  else
    Mirror.println("Alarm notification failed");
}

//Toggle the LED on timer expiration
//trigger the timer every 2 seconds with the command:
//timer repeat 2000
void timer_handler(void)
{
  Dash.toggleLED();
}

//On received SMS, print the sender, timestamp and message
//Send a message to the cloud that an SMS was received with the sender number
//as the content and the tag SMSRX
void cloud_sms(const String &sender, const rtc_datetime_t &timestamp, const String &message) {
  Mirror.println("CLOUD SMS RECEIVED:");
  Mirror.print("SMS SENDER: ");
  Mirror.println(sender);
  Mirror.print("SMS TIMESTAMP: ");
  Mirror.println(timestamp);
  Mirror.println("SMS TEXT: ");
  Mirror.println(message);

  if(HologramCloud.sendMessage(sender, "SMSRX")) {
    Mirror.println("SMS received message sent to cloud.");
  } else {
    Mirror.println("Notification send failed.");
    Mirror.println("Check failure reason by typing:");
    Mirror.println("cloud status");
  }
}

#define SIZE_INBOUND 4096
char buffer_inbound[SIZE_INBOUND];

void cloud_inbound(int length) {
  buffer_inbound[length] = 0; //NULL terminate the data for printing as a String

  Mirror.print("New inbound data, ");
  Mirror.print(length);
  Mirror.println(" bytes: ");
  Mirror.println(buffer_inbound);
}

void cloud_notify(cloud_event e) {
  switch(e) {
    case CLOUD_EVENT_CONNECTED:
      Mirror.println("Connected to Cloud");
      break;
    case CLOUD_EVENT_DISCONNECTED:
      Mirror.println("Disconnected from Cloud");
      break;
    case CLOUD_EVENT_UNREGISTERED:
      Mirror.println("Unregistered from Network");
      break;
    case CLOUD_EVENT_REGISTERED:
      Mirror.println("Registered on Network");
      break;
  }
}

void cloud_location(const rtc_datetime_t &timestamp, const String &lat, const String &lon, int altitude, int uncertainty) {
  Mirror.print("Location: ");
  Mirror.println(timestamp);
  Mirror.print(lat);
  Mirror.print(",");
  Mirror.println(lon);
  Mirror.print("Within ");
  Mirror.print(uncertainty);
  Mirror.println(" meters");
  Mirror.print("Altitude: ");
  Mirror.print(altitude);
  Mirror.println(" meters");
}

void charge_notify(charge_status status) {
  switch(status) {
    case CHARGE_STATUS_FAULT:
      Mirror.println("Charge Fault!!!");
      break;
    case CHARGE_STATUS_CHARGING:
      Mirror.println("Charging");
      break;
    case CHARGE_STATUS_LOW_BATTERY:
      Mirror.print("Low battery");
      break;
    case CHARGE_STATUS_CHARGED:
      Mirror.println("Charged");
      break;
    case CHARGE_STATUS_NO_BATTERY:
      Mirror.println("No battery");
      break;
    case CHARGE_STATUS_NO_INPUT:
      Mirror.println("No input power");
      break;
  }
}

void setup() {
  Mirror.add(Serial);
  Mirror.add(Serial0);
  Mirror.add(Serial2);
  Mirror.begin(115200);
  DashReadEvalPrint.begin();                            //Initialize ReadEvalPrint
  HologramCloud.attachHandlerCharge(charge_notify);     //Handle charge state changes
  Clock.attachAlarmInterrupt(alarm_handler);            //Handle RTC alarm
  Dash.attachTimer(timer_handler);                      //Handle timer expiration
  HologramCloud.attachHandlerSMS(cloud_sms);            //Handle received SMS
  HologramCloud.attachHandlerInbound(cloud_inbound,
                                     buffer_inbound,
                                     SIZE_INBOUND-1);   //Handle received data message
  HologramCloud.attachHandlerNotify(cloud_notify);      //Handle event notifications from cloud
  HologramCloud.attachHandlerLocation(cloud_location);  //Handle location data from cloud
}

void loop() {
  //open the Serial Monitor and type help for a list of commands
  DashReadEvalPrint.run(Mirror);                        //Run the interpreter using USB Serial
}
