#include "Resources.h"
#include "WebServerHooks.h"
#include <HardwareSerial.h>
#include <LittleFS.h>


Resources::VeryEarlyInit::VeryEarlyInit()
{
    Serial.begin(115200, SERIAL_8N1, SERIAL_FULL);
    Serial.println("\n\n\n");
    Serial.println("Resources::VeryEarlyInit::VeryEarlyInit ...");
    Serial.println("Resources::VeryEarlyInit::VeryEarlyInit done");
}


Resources::EarlyInit::EarlyInit(Resources &r)
{
    Serial.println("Resources::EarlyInit::EarlyInit ...");
    r.cncSerial.begin(115200, SWSERIAL_8N1, D4, D3);
    Serial.println("Resources::EarlyInit::EarlyInit done");
}


void Resources::setup()
{
    Serial.println("Firmware::setup ...");
    operatingMode.switchState(OperatingState::State::Setup);

    Serial.println("display init ...");
    display.screen.init();
    display.screen.setFont(ArialMT_Plain_10);
    display.screen.flipScreenVertically();
    display.screen.setLogBuffer(5, 14);
    Serial.println("display ready");

    Serial.println("wifi init ...");
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
    Serial.println("wifi ready");

    display.screen.clear();
    display.screen.drawString(0, Display::L1,
                              "IP: " + String(WiFi.localIP()[0]) + "." + String(WiFi.localIP()[1]) + "...");
    display.screen.drawString(0, Display::L2,
                              "..." + String(WiFi.localIP()[2]) + "." + String(WiFi.localIP()[3]));
    display.screen.display();

    Serial.println("LittleFS ...");
    if(!LittleFS.begin())
    {
        Serial.println("LittleFS failed to mount");
    }
    else
    {
        Serial.println("LittleFS done");
    }

    WebServerHooks::setup(*this);
    webServer.begin();


    Serial.println("Firmware::setup done");
    operatingMode.switchState(OperatingState::State::WaitingForCncController);
}
