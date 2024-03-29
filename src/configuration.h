#pragma once

// ========== section serial ==========

// The serial monitor connection uses the built-in hardware serial controller.
#define SERIAL_MONITOR_BAUD_RATE 115200
#define SERIAL_MONITOR_CONFIG SERIAL_8N1 // see HardwareSerial.h
#define SERIAL_MONITOR_MODE SERIAL_FULL  // see HardwareSerial.h

// The connection to CNC controller uses a software serial.
#define SERIAL_CNC_LINK_BAUD_RATE 38400     // reasonable rates: http://wormfood.net/avrbaudcalc.php
#define SERIAL_CNC_LINK_CONFIG SWSERIAL_8N1 // see SoftwareSerial.h
#define SERIAL_CNC_LINK_RX_PIN D7           // see pins_arduino.h
#define SERIAL_CNC_LINK_TX_PIN D6           // see pins_arduino.h

// ========== section Grbl controller ==========
#define GRBL_CMD_RESET 'Q'                    // see Grbl source: grbl/config.h
#define GRBL_CMD_STATUS_REPORT '?'            // see Grbl source: grbl/config.h
#define GRBL_RESTORE_EEPROM_WIPE_ALL "$RST=*" // see Grbl source : grbl / config.h

// ========== section user interface ==========

#define WEB_SERVER_PORT_NUMBER 80

#define DISPLAY_I2C_ADDRESS 0x3C        // see display specification
#define DISPLAY_I2C_SDA_PIN SDA         // see pins_arduino.h
#define DISPLAY_I2C_SCL_PIN SCL         // see pins_arduino.h
#define DISPLAY_GEOMETRY GEOMETRY_64_48 // see OLEDDisplay.h

// ========== section G-code ==========
// minimum separation for cyclic checks (real time span is >= delay)

#define DELAY_MS_FOR_NEXT_LINE_FROM_FILE_CHECK 80      // separation to send next line to CNC controller
#define DELAY_MS_FOR_CNC_CONTROLLER_IS_READY_CHECK 300 // separation to wait for the CNC controller checks during setup
#define DELAY_MS_FOR_MOTION_FINISHED_CHECK 80          // separation to check if current motion has finished
