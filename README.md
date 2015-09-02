# konekt-dash-arduino-integration
Files necessary to add Konekt Dash and Konekt Dash Pro boards to the Arduino IDE

Needs Arduino IDE version 1.6.5 and up.

To use:

1. Install Arduino IDE 1.6.5 or later, available here: http://arduino.cc
2. Launch Arduino IDE
3. In the Arduino IDE, select `Tools` > `Board` > `Boards Manager`
4. Click on `Arduino SAM Boards (32-bits ARM Cortex-M3)` and select Version `1.6.4` or later, and then click `Install` (this will install the appropriate GCC compiler tools; note that we use a Cortex-M4 series MCU, but don't let that fool you---you will still get the correct compiler tools)
5. Copy/extract the files here in this GitHub repo into your Arduino installation's hardware folder (the `konekt.io` folder should be placed directly beneath the `hardware` folder)
6. Exit Arduino IDE completely and restart Arduino IDE
7. Select `Tools` > `Programmer` > `konekt.io loader`
8. Select `Boards` > `Dash` (or `DashPro`) and start programming!

Questions? Send us a support request on our website: http://konekt.io
