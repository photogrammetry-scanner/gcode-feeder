# Gcode Feeder

A simple G-code feeder that can be accessed wirelessly.
G-codes can be sent manually by the http:// API or automatically from a G-code file.
Example application: [thingiverse.com/thing:3960533](https://www.thingiverse.com/thing:3960533)

<img width=30% src="https://github.com/photogrammetry-scanner/docs/blob/main/images/overview.png" />


**Note:**
- This firmware is developed for the Wemos D1 mini with attached OLED display shield and the GRBL CNC firmware.
- see also this [docs](https://github.com/photogrammetry-scanner/docs/)

## Description

The G-code feeder communicates by serial connection with the CNC controller (Arduino UNO + GRBL firmware).
The communication stream consists of standard G-codes from feeder and GRBL responses from CNC the controller.
The feeder provides a simple user interface by http:// that can be invoked with get parameters which are responded in JSON format.
The UI is tailored for manual invokation but also for scripting.
It allows to send G-codes manually or by automatic processing from G-code file stored on the microcontrollers file system.
In case of file processing, each line is sent to the controller but postponed until the motion has stopped.

**Build + Installation**

Prerequisites:

1. platformio intalled
2. Wemos D1 mini
3. OLED shield
4. wireless LAN

Bild / test / flash:

```bash
# unit tests
platformio test --environment native

# build + flash gcode feeder (this repository)
platformio run --environment d1 --target uploadfs
platformio run --environment d1 --target upload
pio device monitor

# build + flash grbl (repository: https://github.com/photogrammetry-scanner/grbl)
make all
avrdude -c arduino -P /dev/ttyACM0 -p atmega328p -B 10 -F -U flash:w:grbl.hex:i
```

On first installation the wireless manager will open an access point with captive portal to configure the
username/passwort for the local wireless lan.

Once the controller is connected as station to WLAN, the IP-address for accessing the http::// UI will be shown on the display.

## Features

- web based UI (needs wireless AP; no keyboard input etc.)
  - wifi manager for first activation
- API via http:// (for scripting or manual invocation)
  - GET request
  - JSON response
- file manipulation
    - view content
    - upload (POST, stream)
    - delete
    - run gcode from file
- retrieve controller status
    - all ESP controler information
    - currently executed gcode state (transmission, response, error code, motion finished, ...)
- minor status information on display
