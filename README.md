# Gcode Feeder

A simple G-code feeder that can be accessed wirelessly.
G-codes can be sent manually by the http:// api or automatically from a G-code file.
Example application: [thingiverse.com/thing:3960533](https://www.thingiverse.com/thing:3960533)

<img width=30% src="https://github.com/photogrammetry-scanner/docs/blob/main/images/overview.png" />


**Note:**
- This firmware is tailored for the Wemos D1 mini with attached OLED display shield and the GRBL CNC firmware.
- see also this [docs](https://github.com/photogrammetry-scanner/docs/)

**Description**

The G-code feeder communicates by serial connection with the CNC controller (Arduino UNO + GRBL firmware).
The communication stream consists of standard G-codes from feeder and GRBL responses from CNC the controller.
The feeder provides a simplistic user interface by http://.
The UI/API allows to send G-codes manually or by automatic processing from G-code file stored on the microcontrollers
file system.
In case of file processing, each line is sent to the controller but postponed until the motion has stopped.

**Build + Installation**

Prerequisites:

1. platformio intalled
2. Wemos D1 mini
3. OLED shield
4. wireless LAN

Bild and flash:

```bash
pio run --target uploadfs
pio run --target upload
pio device monitor
```

On first installation the wireless manager will open an access point with captive portal to configure the
username/passwort for the local wireless lan.

Once the controller is connected to WLAN, the ip-address for accessing the http::// UI will be shown on the display.
