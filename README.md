# konekt-dash-arduino-integration
Files necessary to add Konekt Dash and Konekt Dash Pro boards to the Arduino IDE

Needs Arduino IDE version 1.6.5 and up.

## Installation
**To set up the Arduino IDE environment for working with the Konekt Dash and Konekt Dash Pro boards, please see our tutorial here: https://content.konekt.io/tutorials/hardware/konekt-dash/getting-started/**

In summary:

1. Install Arduino IDE 1.6.5 or later, available here: https://www.arduino.cc/en/Main/Software
2. Launch Arduino IDE
3. Under `File` > `Preferences`, add our package index URL under `Additional Boards Manager URLs`: http://downloads.konekt.io/arduino/package_konekt_index.json
4. After exiting the preferences screen, select `Tools` > `Board` > `Boards Manager`
5. Under `Type`, select `Contributed`
6. Select the entry for `Konekt` and then select `Install`

## Programming Instructions
**Please see our tutorial here: https://content.konekt.io/tutorials/hardware/konekt-dash/getting-started/**

Contained here are instructions for programming the Dash Beta and Dash Pro Beta via Arduino IDE and USB. Be sure to complete the installation instructions (above) first.

### Dash Beta programming instructions
1. Launch Arduino IDE (version 1.6.5 or later)
2. Select `Boards` > `Dash` (or `DashPro`)
3. Select `Tools` > `Programmer`, and select `konekt.io loader` (for USB programming) or `konekt.io OTA programmer` (for over-the-air cellular programming) 
4. Write your sketch (program) and save the sketch file (see examples)
5. Select the `Verify` button in the Arduino IDE
6. **If programming via USB:** power-cycle the Konekt Dash (places it in programming mode) and select `Upload` in the Arduino IDE within 10 seconds of power-cycling (the Konekt Dash will flash with a two-blink heartbeat pattern while in programming mode, and will timeout if no program upload begins)
6. **If programming over-the-air via cellular:** For over-the-air (OTA) programming, the Dash MUST NOT be placed into USB programming mode using the PGM button. Instead, simply select `Upload` and follow the on-screen instructions. Your API key can be obtained from: https://dashboard.konekt.io/apikey


## For more information
Example Arduino sketches can be found at: https://github.com/konektlabs/konekt-dash-arduino-examples

Check out our tutorial: https://content.konekt.io/tutorials/hardware/konekt-dash/getting-started/

Questions? Browse answers or ask a question at: https://community.konekt.io