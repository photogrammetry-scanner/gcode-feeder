#pragma once

#include <ESP8266WiFi.h>
#include <Esp.h>

// operation modes
#include "OperatingState.h"

// display
#include "display/Display.h"

// wifi manager + web server
#include <DNSServer.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>

// serial
#include "serial/LineBufferedStream.h"
#include <SoftwareSerial.h>

// gcode
#include "GcodeBuffer.h"
#include "GcodeFileRunner.h"

// hardcoded config
#include "configuration.h"


typedef struct Resources
{
    void setup();

    struct PreInit
    {
        PreInit();
    } _preInit = {};

    OperatingState operatingMode{};

    DNSServer dnsServer{};
    AsyncWebServer webServer{ WEB_SERVER_PORT_NUMBER };

    SoftwareSerial cncSerial{};
    LineBufferedStream cncSerialBuffer{ cncSerial };

    Display display{};

    GcodeBuffer gcodeBuffer{};
    GcodeFileRunner gcodeFileRunner{ gcodeBuffer, operatingMode };

    struct PostInit
    {
        PostInit(Resources &r);
    } _postInit = { *this };

} Resources;
