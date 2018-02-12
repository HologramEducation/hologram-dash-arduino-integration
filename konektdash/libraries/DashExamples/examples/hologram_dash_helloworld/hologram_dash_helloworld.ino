/* Hologram Dash Hello World
*
* Purpose: This program demonstrates interactive serial mode,
* a mechanism for performing cable replacement serial passthrough
* over cellular to the cloud. This example works out-of-the-box
* with zero configuration.
*
* License: Copyright (c) 2017 Konekt, Inc. All Rights Reserved.
*
* Released under the MIT License (MIT)
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
*
*/

//Process a received SMS
void cloud_sms(const String &sender, const rtc_datetime_t &timestamp, const String &message) {
  //Echo the received SMS back into the cloud
  Serial.print("From: ");
  Serial.print(sender);
  Serial.print(" @ ");
  Serial.print(timestamp);
  Serial.println(" Message:");
  Serial.println(message);
}

#define SIZE_INBOUND 4096               //Inbound message size limit
char buffer_inbound[SIZE_INBOUND];      //Holds inbound message

//Process a received inbound message
void cloud_inbound(int length) {
  buffer_inbound[length] = 0; //NULL terminate the data for printing as a String
  Serial.println("Inbound Message: ");
  Serial.println(buffer_inbound);
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
  Serial.begin(); /* USB Serial */

  HologramCloud.attachHandlerSMS(cloud_sms);        /* Use the cloud_sms function to process incoming SMS */

  //Use the cloud_inbound function to process a received message.
  //Uses a buffer and size defined in the sketch for user requirements.
  //If String data, reserve one byte for NULL-terminator
  //The full buffer size can be used for binary or non-printed data.
  HologramCloud.attachHandlerInbound(cloud_inbound,
                                     buffer_inbound,
                                     SIZE_INBOUND-1);
  
  HologramCloud.listen(4010);

  HologramCloud.attachHandlerNotify(cloud_notify);  /* Get status notifications */

  //wait until Connected
  while(!HologramCloud.isConnected()) {
    Dash.snooze(1000);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("Hologram Dash Hello World Example Started!");
  HologramCloud.sendMessage("Hello, World!"); /* one-time message */
}

void loop() {
  //read the USB serial port for input
  while(Serial.available()) {
    char c = Serial.read();
    HologramCloud.write(c);                   /* Add char to the message */
    if(c == '\n') {                           /* Send message on newline */
      if(HologramCloud.sendMessage()) {
        Serial.println("Message Sent");
      } else {
        Serial.println("Message Send Failed");
      }
    }
  }
}
