# running_wheel_rotary_encoder
This repository contains code to record the rotations of a running wheel using 2 IR-LED + Photodiode sensors to detect wheel rotations. 
This is realized using an ESP32 to record and decode the clockwise and counterclockwise rotations. 
Using a Python program, this data is read and saved as .csv

## Installation ##
1. In this Project an ESP32 with a CP2102 chip is used. This requires the installation of the appropriate USB-drive:
"https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers?tab=downloads"

2. Futher tthe Arduino IDE is required to upload the code to the ESP32:
"https://www.arduino.cc/en/software"

3. The ESP32 Board need to be added to Arduino IDE. See instructions here: "https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/"
In short: Go to "File>Preferences", then add "https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json" under “Additional Board Manager URLs”
