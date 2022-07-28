#if !defined(ENV_NATIVE)
#include "Resources.h"
#include "WebServerHooks.h"
#include <ArduinoOTA.h>
#include <HardwareSerial.h>
#include <LittleFS.h>
#include <WiFiUdp.h>

#define xstr(s) str(s)
#define str(s) #s

Resources::PreInit::PreInit()
{
    while(millis() < 100)
        EspClass::wdtFeed();
    Serial.begin(SERIAL_MONITOR_BAUD_RATE, SERIAL_MONITOR_CONFIG, SERIAL_MONITOR_MODE);
    Serial.println("\n\n\n");
    while(millis() < 400)
    {
        EspClass::wdtFeed();
        delayMicroseconds(10000);
        Serial.print('.');
    }
    Serial.println();
    Serial.println(std::string(std::to_string(millis()) + " initialized hardware-serial: baud=" + xstr(SERIAL_MONITOR_BAUD_RATE) +
                               ", config=" + xstr(SERIAL_MONITOR_CONFIG) + ", mode=" + xstr(SERIAL_MONITOR_MODE))
                   .c_str());
    Serial.println(std::string(std::to_string(millis()) + " Resources::PreInit::PreInit done").c_str());
}


namespace
{
void setupDisplay(Resources &r)
{
    Serial.println(std::string(std::to_string(millis()) + " display init ...").c_str());
    r.display.setup();
    Serial.println(std::string(std::to_string(millis()) + " display ready").c_str());
}


void setupWifi(Resources &r)
{
    Serial.println(std::string(std::to_string(millis()) + " wifi init ...").c_str());
    AsyncWiFiManager wifiManager(&r.webServer, &r.dnsServer);
    wifiManager.setDebugOutput(false);
    wifiManager.setAPStaticIPConfig(IPAddress(1, 1, 1, 1), IPAddress(1, 1, 1, 1), IPAddress(255, 255, 255, 0));

    r.display.printLine(Display::L1, "wifi not");
    r.display.printLine(Display::L2, "configured");
    r.display.printLine(Display::L3, "ConfigAP:");
    r.display.printLine(Display::L4, "AutoConfigAP");
    r.display.printLine(Display::L5, "IP: 1.1.1.1");
    r.display.displayIfTouched();
    wifiManager.autoConnect("AutoConfigAP");
    Serial.println(std::string(std::to_string(millis()) + " wifi ready").c_str());

    r.display.clear();
    r.display.printLine(Display::L1, "IP: " + std::to_string(WiFi.localIP()[0]) + "." + std::to_string(WiFi.localIP()[1]) + "...");
    r.display.printLine(Display::L2, "..." + std::to_string(WiFi.localIP()[2]) + "." + std::to_string(WiFi.localIP()[3]));
    r.display.displayIfTouched();
}

void setupWebServiceHooks(Resources(&r))
{
    WebServerHooks::setup(r);
    r.webServer.begin();
}

void setupOtaUpdateService()
{
    ArduinoOTA.onStart(
    []()
    {
        std::string type;
        if(ArduinoOTA.getCommand() == U_FLASH)
        {
            type = "sketch";
        }
        else
        {
            LittleFS.end();
            type = "filesystem";
        }
        Serial.println(std::string(std::to_string(millis()) + " updating " + type).c_str());
    });

    ArduinoOTA.onEnd([]() { Serial.println(std::string(std::to_string(millis()) + " update finished").c_str()); });
    ArduinoOTA.onProgress(
    [](unsigned int progress, unsigned int total)
    {
        Serial.println(std::string(std::to_string(millis()) + " progress: " + std::to_string(progress / (total / 100))).c_str());
    });
    ArduinoOTA.onError(
    [](ota_error_t error)
    {
        Serial.println(std::string(std::to_string(millis()) + "error: " + std::to_string(error)).c_str());
        if(error == OTA_AUTH_ERROR)
        {
            Serial.println(std::string(std::to_string(millis()) + " auth failed").c_str());
        }
        else if(error == OTA_BEGIN_ERROR)
        {
            Serial.println(std::string(std::to_string(millis()) + " begin failed").c_str());
        }
        else if(error == OTA_CONNECT_ERROR)
        {
            Serial.println(std::string(std::to_string(millis()) + " connect failed").c_str());
        }
        else if(error == OTA_RECEIVE_ERROR)
        {
            Serial.println(std::string(std::to_string(millis()) + " receive failed").c_str());
        }
        else if(error == OTA_END_ERROR)
        {
            Serial.println(std::string(std::to_string(millis()) + " end failed").c_str());
        }
    });
    ArduinoOTA.begin();
    Serial.print(std::string(std::to_string(millis()) + " OTA ready: ").c_str());
    Serial.println(WiFi.localIP());
}


void setupFilesystem()
{
    Serial.println(std::string(std::to_string(millis()) + " LittleFS ...").c_str());
    if(!LittleFS.begin())
    {
        Serial.println(std::string(std::to_string(millis()) + " LittleFS failed to mount").c_str());
    }
    else
    {
        Serial.println(std::string(std::to_string(millis()) + " LittleFS done").c_str());
    }
}


void setupCncSerial(Resources &r)
{
    Serial.println(std::string(std::to_string(millis()) + " Resources::PostInit::PostInit ...").c_str());
    r.cncSerial.begin(SERIAL_CNC_LINK_BAUD_RATE, SERIAL_CNC_LINK_CONFIG, SERIAL_CNC_LINK_RX_PIN, SERIAL_CNC_LINK_TX_PIN);
    Serial.println(std::string(std::to_string(millis()) + " initialized cnc controller software-serial: baud=" +
                               xstr(SERIAL_CNC_LINK_BAUD_RATE) + ", config=" + xstr(SERIAL_CNC_LINK_CONFIG) +
                               ", rx_pin=" + xstr(SERIAL_CNC_LINK_RX_PIN) + ", tx_pin=" + xstr(SERIAL_CNC_LINK_TX_PIN))
                   .c_str());
    Serial.println(std::string(std::to_string(millis()) + " Resources::PostInit::PostInit done").c_str());
}
} // namespace


void Resources::setup()
{
    Serial.println(std::string(std::to_string(millis()) + " Firmware::setup ...").c_str());
    operatingMode.switchState(OperatingState::State::DoSetup);

    setupDisplay(*this);
    setupWifi(*this);
    setupWebServiceHooks(*this);
    setupOtaUpdateService();
    setupFilesystem();
    setupCncSerial(*this);

    Serial.println(std::string(std::to_string(millis()) + " Firmware::setup done").c_str());
    operatingMode.switchState(OperatingState::State::SetupFinished);
}

#endif
