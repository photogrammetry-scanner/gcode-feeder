#if !defined(ENV_NATIVE)
#include "Resources.h"
#include "WebServerHooks.h"
#include <HardwareSerial.h>
#include <LittleFS.h>

#include <ArduinoOTA.h>
#include <WiFiUdp.h>

#define xstr(s) str(s)
#define str(s) #s

Resources::PreInit::PreInit()
{
    WiFi.mode(WIFI_OFF);
    Serial.begin(SERIAL_MONITOR_BAUD_RATE, SERIAL_MONITOR_CONFIG, SERIAL_MONITOR_MODE);
    Serial.println("\n\n\n");
    Serial.println(std::string(std::to_string(millis()) + " Resources::PreInit::PreInit ...").c_str());
    Serial.println(std::string(std::to_string(millis()) + " initialized hardware-serial: baud=" + xstr(SERIAL_MONITOR_BAUD_RATE) +
                               ", config=" + xstr(SERIAL_MONITOR_CONFIG) + ", mode=" + xstr(SERIAL_MONITOR_MODE))
                   .c_str());
    Serial.println(std::string(std::to_string(millis()) + " Resources::PreInit::PreInit done").c_str());
}


Resources::PostInit::PostInit(Resources &r)
{
    Serial.println(std::string(std::to_string(millis()) + " Resources::PostInit::PostInit ...").c_str());
    r.cncSerial.begin(SERIAL_CNC_LINK_BAUD_RATE, SERIAL_CNC_LINK_CONFIG, SERIAL_CNC_LINK_RX_PIN, SERIAL_CNC_LINK_TX_PIN);
    Serial.println(std::string(std::to_string(millis()) + " initialized cnc controller software-serial: baud=" +
                               xstr(SERIAL_CNC_LINK_BAUD_RATE) + ", config=" + xstr(SERIAL_CNC_LINK_CONFIG) +
                               ", rx_pin=" + xstr(SERIAL_CNC_LINK_RX_PIN) + ", tx_pin=" + xstr(SERIAL_CNC_LINK_TX_PIN))
                   .c_str());
    Serial.println(std::string(std::to_string(millis()) + " Resources::PostInit::PostInit done").c_str());
}


void Resources::setup()
{

    Serial.println(std::string(std::to_string(millis()) + " Firmware::setup ...").c_str());
    operatingMode.switchState(OperatingState::State::Setup);

    [&]() // display
    {
        Serial.println(std::string(std::to_string(millis()) + " display init ...").c_str());
        display.screen.init();
        display.screen.setFont(ArialMT_Plain_10);
        display.screen.flipScreenVertically();
        display.screen.setLogBuffer(6, 14);
        display.screen.clear();
        display.screen.display();
        Serial.println(std::string(std::to_string(millis()) + " display ready").c_str());
    }();

    [&]() // wifi
    {
        Serial.println(std::string(std::to_string(millis()) + " wifi init ...").c_str());
        AsyncWiFiManager wifiManager(&webServer, &dnsServer);
        wifiManager.setDebugOutput(false);
        wifiManager.setAPStaticIPConfig(IPAddress(1, 1, 1, 1), IPAddress(1, 1, 1, 1), IPAddress(255, 255, 255, 0));

        display.screen.drawString(0, Display::L1, "wifi not");
        display.screen.drawString(0, Display::L2, "configured");
        display.screen.drawString(0, Display::L3, "ConfigAP:");
        display.screen.drawString(0, Display::L4, "AutoConfigAP");
        display.screen.drawString(0, Display::L5, "IP: 1.1.1.1");
        display.screen.display();
        wifiManager.autoConnect("AutoConfigAP");
        Serial.println(std::string(std::to_string(millis()) + " wifi ready").c_str());

        display.screen.clear();
        display.screen.drawString(
        0, Display::L1,
        std::string("IP: " + std::to_string(WiFi.localIP()[0]) + "." + std::to_string(WiFi.localIP()[1]) + "...").c_str());
        display.screen.drawString(
        0, Display::L2, std::string("..." + std::to_string(WiFi.localIP()[2]) + "." + std::to_string(WiFi.localIP()[3])).c_str());
        display.screen.display();
    }();

    [&]() // file system
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
    }();

    [&]() // web service
    {
        WebServerHooks::setup(*this);
        webServer.begin();
    }();

    [&]() // over the air firmware update: platformio run --target upload --upload-port <ip-address>
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
            Serial.println(
            std::string(std::to_string(millis()) + " progress: " + std::to_string(progress / (total / 100))).c_str());
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
    }();

    Serial.println(std::string(std::to_string(millis()) + " Firmware::setup done").c_str());
    operatingMode.switchState(OperatingState::State::SetupFinished);
}

#endif
