#pragma once

#include <ESP8266WiFi.h>
#include <Esp.h>

// operation modes
#include "OperatingState.h"

// display
#include "Display.h"

// wifi manager + web server
#include <DNSServer.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>

// serial
#include "LineBufferedStream.h"
#include <SoftwareSerial.h>

// gcode
#include "GcodeBuffer.h"
#include "GcodeFileRunner.h"


typedef struct Resources
{
    void setup();

    struct VeryEarlyInit
    {
        VeryEarlyInit();
    } _veryEarlyInit = {};

    OperatingState operatingMode{};
    DNSServer dnsServer{};
    AsyncWebServer webServer{ 80 };
    SoftwareSerial cncSerial{};
    LineBufferedStream cncSerialBuffer{ cncSerial };
    Display display;

    GcodeBuffer gcodeBuffer;
    GcodeFileRunner gcodeFileRunner{ gcodeBuffer, operatingMode };

    struct EarlyInit
    {
        EarlyInit(Resources &r);
    } _earlyInit = { *this };

} Resources;
