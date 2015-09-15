# konekt-dash-arduino-integration
Files necessary to add Konekt Dash and Konekt Dash Pro boards to the Arduino IDE

Needs Arduino IDE version 1.6.5 and up.

## Installation
To set up the Arduino IDE environment for working with the Konekt Dash and Konekt Dash Pro boards:

1. Install Arduino IDE 1.6.5 or later, available here: http://arduino.cc
2. Launch Arduino IDE
3. In the Arduino IDE, select `Tools` > `Board` > `Boards Manager`
4. Click on `Arduino SAM Boards (32-bits ARM Cortex-M3)` and select Version `1.6.4` or later, and then click `Install` (this will install the appropriate GCC compiler tools; note that we use a Cortex-M4 series MCU, but don't let that fool you---you will still get the correct compiler tools)
5. Copy/extract the files here in this GitHub repo into your Arduino installation's hardware folder (the `konekt.io` folder should be placed directly beneath the `hardware` folder)
6. Exit Arduino IDE completely and restart Arduino IDE
7. Select `Boards` > `Dash` (or `DashPro`) and start programming (see below for instructions)!
8. Select `Tools` > `Programmer` > `konekt.io loader`

## Programming Instructions

Contained here are instructions for programming the Dash Beta and Dash Pro Beta via Arduino IDE and USB. Be sure to complete the installation instructions (above) first.

### Dash Beta programming instructions
1. Launch Arduino IDE (version 1.6.5 or later)
2. Select `Boards` > `Dash` (or `DashPro`) and start programming (see below for instructions)!
3. Select `Tools` > `Programmer` > `konekt.io loader`
4. Write your sketch (program) and save the sketch file (see examples)
5. Select the `Verify` button in the Arduino IDE
6. Power-cycle the Konekt Dash (places it in programming mode) and select `Upload` in the Arduino IDE within 10 seconds of power-cycling (the Konekt Dash will flash with a two-blink heartbeat pattern while in programming mode, and will timeout if no program upload begins)

### Dash Pro Beta programming instructions
1. Launch Arduino IDE (version 1.6.5 or later)
2. Select `Boards` > `Dash` (or `DashPro`) and start programming (see below for instructions)!
3. Select `Tools` > `Programmer` > `konekt.io loader`
4. Write your sketch (program) and save the sketch file (see examples)
5. Select the `Verify` button in the Arduino IDE
6. Press the tiny button on the Konekt Dash Pro nearest to the "U-blox" module (places it in programming mode) and select `Upload` in the Arduino IDE.

## For more information
Example Arduino sketches can be found at: https://github.com/konektlabs/konekt-dash-arduino-examples 

Questions? Chat about Dash Beta and Dash Pro Beta board experiences at: https://community.konekt.io/t/dash-beta-experiences-and-troubleshooting/75/4 
