#if !defined(ENV_NATIVE)
#include "Resources.h"
#include "WebServerHooks.h"
#include <HardwareSerial.h>
#include <LittleFS.h>

Resources::PreInit::PreInit()
{
    WiFi.mode(WIFI_OFF);
    Serial.begin(SERIAL_MONITOR_BAUD_RATE, SERIAL_MONITOR_CONFIG, SERIAL_MONITOR_MODE);
    Serial.println("\n\n\n");
    Serial.println(std::string(std::to_string(millis()) + " Resources::PreInit::PreInit ...").c_str());
    while(millis() < 125)
        ;
    Serial.println("Resources::PreInit::PreInit done");
}


Resources::PostInit::PostInit(Resources &r)
{
    Serial.println(std::string(std::to_string(millis()) + " Resources::PostInit::PostInit ...").c_str());
    r.cncSerial.begin(SERIAL_CNC_LINK_BAUD_RATE, SERIAL_CNC_LINK_CONFIG, SERIAL_CNC_LINK_RX_PIN, SERIAL_CNC_LINK_TX_PIN);
    Serial.println(std::string(std::to_string(millis()) + " Resources::PostInit::PostInit done").c_str());
}


void Resources::setup()
{
    Serial.println(std::string(std::to_string(millis()) + " Firmware::setup ...").c_str());
    operatingMode.switchState(OperatingState::State::Setup);

    Serial.println(std::string(std::to_string(millis()) + " display init ...").c_str());
    display.screen.init();
    display.screen.setFont(ArialMT_Plain_10);
    display.screen.flipScreenVertically();
    display.screen.setLogBuffer(6, 14);
    Serial.println(std::string(std::to_string(millis()) + " display ready").c_str());

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

    Serial.println(std::string(std::to_string(millis()) + " LittleFS ...").c_str());
    if(!LittleFS.begin())
    {
        Serial.println(std::string(std::to_string(millis()) + " LittleFS failed to mount").c_str());
    }
    else
    {
        Serial.println(std::string(std::to_string(millis()) + " LittleFS done").c_str());
    }

    WebServerHooks::setup(*this);
    webServer.begin();


    Serial.println(std::string(std::to_string(millis()) + " Firmware::setup done").c_str());
    operatingMode.switchState(OperatingState::State::WaitingForCncControllerReady);
}

#endif
