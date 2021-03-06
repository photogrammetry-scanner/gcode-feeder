#pragma once

// ========== section serial ==========

// The serial monitor connection uses the built-in hardware serial controller.
#define SERIAL_MONITOR_BAUD_RATE 115200
#define SERIAL_MONITOR_CONFIG SERIAL_8N1 // see HardwareSerial.h
#define SERIAL_MONITOR_MODE SERIAL_FULL  // see HardwareSerial.h

// The connection to CNC controller uses a software serial.
#define SERIAL_CNC_LINK_BAUD_RATE 28800     // reasonable rates: 28800, 57600, 115200 (decrease on xmission errors)
#define SERIAL_CNC_LINK_CONFIG SWSERIAL_8N1 // see SoftwareSerial.h
#define SERIAL_CNC_LINK_RX_PIN D4           // see pins_arduino.h
#define SERIAL_CNC_LINK_TX_PIN D3           // see pins_arduino.h

// ========== section user interface ==========

#define WEB_SERVER_PORT_NUMBER 80

#define DISPLAY_I2C_ADDRESS 0x3C        // see display specification
#define DISPLAY_I2C_SDA_PIN SDA         // see pins_arduino.h
#define DISPLAY_I2C_SCL_PIN SCL         // see pins_arduino.h
#define DISPLAY_GEOMETRY GEOMETRY_64_48 // see OLEDDisplay.h

// ========== section G-code ==========
// minimum separation for cyclic checks (real time span is >= delay)

#define DELAY_MS_FOR_NEXT_LINE_FROM_FILE_CHECK 80      // separation to send next line to CNC controller
#define DELAY_MS_FOR_CNC_CONTROLLER_IS_READY_CHECK 500 // separation to wait for the CNC controller to boot
#define DELAY_MS_FOR_MOTION_FINISHED_CHECK 100         // separation to check if current motion has finished
