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
    Serial.println("Resources::PreInit::PreInit ...");
    while(millis() < 125)
        ;
    Serial.println("Resources::PreInit::PreInit done");
}


Resources::PostInit::PostInit(Resources &r)
{
    Serial.println("Resources::PostInit::PostInit ...");
    r.cncSerial.begin(SERIAL_CNC_LINK_BAUD_RATE, SERIAL_CNC_LINK_CONFIG, SERIAL_CNC_LINK_RX_PIN, SERIAL_CNC_LINK_TX_PIN);
    Serial.println("Resources::PostInit::PostInit done");
}


void Resources::setup()
{
    Serial.println("Firmware::setup ...");
    operatingMode.switchState(OperatingState::State::Setup);

    Serial.println("display init ...");
    display.screen.init();
    display.screen.setFont(ArialMT_Plain_10);
    display.screen.flipScreenVertically();
    display.screen.setLogBuffer(6, 14);
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
    display.screen.drawString(0, Display::L1, "IP: " + String(WiFi.localIP()[0]) + "." + String(WiFi.localIP()[1]) + "...");
    display.screen.drawString(0, Display::L2, "..." + String(WiFi.localIP()[2]) + "." + String(WiFi.localIP()[3]));
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

#endif
