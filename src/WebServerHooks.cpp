#include "WebServerHooks.h"
#include "FS.h"
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>
#include <LittleFS.h>
#include <WString.h>

void indexHtml(AsyncWebServerRequest &request, const String &extra_pre_html = "", const String &extra_post_html = "")
{
    Serial.println("WebServerHooks -> indexHtml");

    String info;
    info += "<html>";
    info += "  <h1> Firmware ";
    info += ESP.getSketchMD5();
    info += " size  ";
    info += ESP.getSketchSize();
    info += "  </h1> ";
    if(extra_pre_html.length() > 0)
        info += extra_pre_html;

    info += "  <b>Device</b>";
    info += "  <ul>";
    info += "    <li><a href=/status>device status</a></li>";
    info += "    <li><a href=/reboot>reboot</a></li>";
    info += "    <li><a href=/resetWifi>reset wifi settings</a></li>";
    info += "    <li><a href=/files>list files</a></li>";
    info += "    <li><a href=/file?name=test.g>read file test.g</a></li>";
    info += "  </ul>";

    info += "  <b>CNC Controller (send gcode)</b>";
    info += "  <ul>";
    info += "    <li><a href=/logs>controller logs</a></li>";
    info += "    <li><a href=/sendGcode?line=G91>G91 (relative motion)</a></li>";
    info += "    <li><a href=/sendGcode?line=G1%20X+10%20F2000>G1 X+10 F2000 (move X+)</a></li>";
    info += "    <li><a href=/sendGcode?line=G1%20X-10%20F2000>G1 X-10 F2000 (move X-)</a></li>";
    info += "    <li><a href=/sendGcode?line=G1%20Z+10%20F2000>G1 Z+10 F2000 (move Z+)</a></li>";
    info += "    <li><a href=/sendGcode?line=G1%20Z-10%20F2000>G1 Z-10 F2000 (move Z-)</a></li>";
    info += "    <li><a href=/runFile?name=test.g>run from file 'test.g'</a></li>";
    info += "  </ul>";

    if(extra_post_html.length() > 0)
        info += extra_post_html;
    info += "</html>";
    request.send(200, "text/html", info);
}


void reboot(AsyncWebServerRequest &request)
{
    Serial.println("WebServerHooks -> reboot");
    request.send(200, "text/html", "reboot");
    delay(1000);
    ESP.restart();
}


void deviceStatus(AsyncWebServerRequest &request, const String &extra_pre_html = "", const String &extra_post_html = "")
{
    Serial.println("WebServerHooks -> deviceStatus");
    String status;
    status += "<html>";
    if(extra_pre_html.length() > 0)
        status += extra_pre_html;
    status += "  <table>"
              "    <tr>"
              "      <th>status</th>"
              "      <th>value</th>"
              "    </tr>"
              "    <tr>"
              "      <td>shutdown reason</td>"
              "      <td>";
    status += ESP.getResetReason().c_str();
    status += "      </td>"
              "    </tr>";
    status += "    <tr>"
              "      <td>VCC [mV]</td>"
              "      <td>";
    status += ESP.getVcc();
    status += "      </td>"
              "    </tr>";
    status += "    <tr>"
              "      <td>free heap [B]</td>"
              "      <td>";
    status += ESP.getFreeHeap();
    status += "      </td>"
              "    </tr>";
    status += "    <tr>"
              "      <td>max. free block size </td>"
              "      <td>";
    status += ESP.getMaxFreeBlockSize();
    status += "</td>"
              "    </tr>";
    status += "    <tr>"
              "      <td>heap fragmentation [%]</td>"
              "      <td>";
    status += ESP.getHeapFragmentation();
    status += "      </td>"
              "    </tr>";
    status += "    <tr>"
              "      <td>free continuous stack [B]</td>"
              "      <td>";
    status += ESP.getFreeContStack();
    status += "      </td>"
              "    </tr>";
    status += "    <tr>"
              "      <td>SDK version</td>"
              "      <td>";
    status += ESP.getSdkVersion();
    status += "      </td>"
              "    </tr>";
    status += "    <tr>"
              "      <td>core version</td>"
              "      <td>";
    status += ESP.getCoreVersion();
    status += "      </td>"
              "    </tr>";
    status += "    <tr>"
              "      <td>full version</td>"
              "      <td>";
    status += ESP.getFullVersion();
    status += "</td>"
              "    </tr>";
    status += "    <tr>"
              "      <td>boot loader version</td>"
              "      <td>";
    status += ESP.getBootVersion();
    status += "</td>"
              "    </tr>";
    status += "    <tr>"
              "      <td>boot mode</td>"
              "      <td>";
    status += ESP.getBootMode();
    status += "      </td>"
              "    </tr>";
    status += "    <tr>"
              "      <td>CPU frequency [MHz]</td>"
              "      <td>";
    status += ESP.getCpuFreqMHz();
    status += "      </td>"
              "    </tr>";
    status += "    <tr>"
              "      <td>flash chip ID</td>"
              "      <td>";
    status += ESP.getFlashChipId();
    status += "</td>"
              "    </tr>";
    status += "    <tr>"
              "      <td>flash chip vendor ID</td>"
              "      <td>";
    status += ESP.getFlashChipVendorId();
    status += "      </td>"
              "    </tr>";
    status += "    <tr>"
              "      <td>flash chip real size[B]</td>"
              "      <td>";
    status += ESP.getFlashChipRealSize();

    status += "      </td>"
              "    </tr>";
    status += "    <tr>"
              "      <td>flash chip size [B]</td>"
              "      <td>";
    status += ESP.getFlashChipSize();
    status += "      </td>"
              "    </tr>";
    status += "    <tr>"
              "      <td>flash chip speed [Hz]</td>"
              "      <td>";
    status += ESP.getFlashChipSpeed();
    status += "      </td>"
              "    </tr>";
    status += "    <tr>"
              "      <td>flash chip mode</td>"
              "      <td>";
    status += ESP.getFlashChipMode();
    status += "      </td>"
              "    </tr>";
    status += "    <tr>"
              "      <td>flash chip size by chip ID</td>"
              "      <td>";
    status += ESP.getFlashChipSizeByChipId();
    status += "      </td>"
              "    </tr>";
    status += "    <tr>"
              "      <td>sketch size [B]</td>"
              "      <td>";
    status += ESP.getSketchSize();
    status += "      </td>"
              "    </tr>";

    status += "    <tr>"
              "      <td>sketch MD5</td>"
              "      <td>";
    status += ESP.getSketchMD5();
    status += "      </td>"
              "    </tr>";

    status += "    <tr>"
              "      <td>free sketch space [B]</td>"
              "      <td>";
    status += ESP.getFreeSketchSpace();

    status += "      </td>"
              "    </tr>";
    status += "    <tr>"
              "      <td>reset reason</td>"
              "      <td>";
    status += ESP.getResetReason();
    status += "      </td>"
              "    </tr>";

    status += "    <tr>"
              "      <td>reset info</td>"
              "      <td>";
    status += ESP.getResetInfo();
    status += "      </td>"
              "    </tr>";
    status += "  <table>";
    if(extra_post_html.length() > 0)
        status += extra_post_html;
    status += "</html>";

    request.send(200, "text/html", status);
}


void resetWifi(AsyncWebServerRequest &request,
               AsyncWebServer &webServer,
               DNSServer &dnsServer,
               const String &extra_pre_html = "",
               const String &extra_post_html = "")
{
    Serial.println("WebServerHooks -> resetWifi");
    String info;
    info += "<html>";
    if(extra_pre_html.length() > 0)
        info += extra_pre_html;
    info += "resetting wifi settings and reboot ...";
    if(extra_post_html.length() > 0)
        info += extra_post_html;
    info += "</html>";

    request.send(200, "text/html", info);

    delay(2000);
    AsyncWiFiManager wifiManager(&webServer, &dnsServer);
    wifiManager.setDebugOutput(false);
    wifiManager.resetSettings();
    ESP.restart();
}


void sendGcode(AsyncWebServerRequest &request,
               GcodeBuffer &gcodeBuffer,
               OperatingState &operatingMode,
               const String &extra_pre_html = "",
               const String &extra_post_html = "")
{
    String info;
    Serial.println("WebServerHooks -> sendGcode");

    if(!operatingMode.isState(OperatingState::State::Idle))
    {
        Serial.println("cannot accept gcode via http while not in idle (" + operatingMode.toString() + ")");
        request.send(200, "text/html",
                     "cannot accept gcode via http while not in idle (" + operatingMode.toString() + ")");
        return;
    }

    if(request.hasParam("line"))
    {
        if(request.hasArg("line"))
        {
            String gcode{ request.getParam("line")->value() };
            info += "gcode=" + gcode + "<br/>";

            if(gcodeBuffer.isProcessed())
            {
                Serial.println("buffer gcode='" + gcode + "'");
                gcodeBuffer.setGcode(gcode);
            }
            else
            {
                Serial.println("failed to buffer gcode, skipping '" + gcode + "'");
            }
        }
        else
        {
            Serial.println("failed to retrieve value from http-request");
        }
    }
    else
    {
        info += "get-parameter 'line' not found<br/>";
    }
    request.send(200, "text/html", info);
}


void listFiles(AsyncWebServerRequest &request)
{
    Serial.println("WebServerHooks -> listFiles");

    Serial.printf("listing directory: /\n");
    String dirListing;

    File root = LittleFS.open("/", "r");
    if(!root)
    {
        Serial.println("failed to open directory");
        request.send(200, "text/html", "failed to open directory");
        return;
    }
    if(!root.isDirectory())
    {
        Serial.println("not a directory");
        request.send(200, "text/html", "not a directory");
        return;
    }

    File file = root.openNextFile();
    dirListing += ("<html>");
    while(file)
    {
        if(file.isDirectory())
        {

            dirListing += String() + "dir  " + file.name() + " ";
            time_t t = file.getLastWrite();
            struct tm *tmstruct = localtime(&t);
            dirListing += String() + "date: " + ((tmstruct->tm_year) + 1900) + "." +
                          ((tmstruct->tm_mon) + 1) + "." + tmstruct->tm_mday + " " +
                          "time: " + tmstruct->tm_hour + ":" + tmstruct->tm_min + ":" + tmstruct->tm_sec;
        }
        else
        {
            dirListing += String() + "file " + file.name() + " size: " + file.size() + " ";
            time_t t = file.getLastWrite();
            struct tm *tmstruct = localtime(&t);
            dirListing += String() + "date: " + ((tmstruct->tm_year) + 1900) + "." +
                          ((tmstruct->tm_mon) + 1) + "." + tmstruct->tm_mday + " " +
                          "time: " + tmstruct->tm_hour + ":" + tmstruct->tm_min + ":" + tmstruct->tm_sec;
        }
        file = root.openNextFile();
    }
    dirListing += ("</html>");

    request.send(200, "text/html", dirListing);
}


void readFile(AsyncWebServerRequest &request)
{
    Serial.println("WebServerHooks -> readFile");
    String path = "NA";

    if(request.hasArg("name"))
        path = request.getParam("name")->value();

    if(!LittleFS.exists(path))
    {
        Serial.println("failed to read file '" + path + "', file does not exist");
        request.send(200, "text/html", "failed to read file name='" + path + "', file does not exist");
        return;
    }

    Serial.println("read file '" + path + "'");
    request.send(LittleFS, path, "text/plain");
}


void runFile(AsyncWebServerRequest &request, GcodeFileRunner &fileRunner, OperatingState &operatingMode)
{
    Serial.println("WebServerHooks -> runFile");
    String path = "NA";

    if(!operatingMode.isState(OperatingState::State::Idle))
    {
        Serial.println("cannot start processing file while not in idle (" + operatingMode.toString() + ")");
        request.send(200, "text/html",
                     "cannot start processing file while not in idle(" + operatingMode.toString() + ")");
        return;
    }

    if(request.hasArg("name"))
        path = request.getParam("name")->value();

    if(!LittleFS.exists(path))
    {
        Serial.println("failed to start processing file '" + path + "', file does not exist");
        request.send(200, "text/html", "failed to start processing file '" + path + "', file does not exist");
        return;
    }

    fileRunner.reset();
    fileRunner.setFilepath(path);
    operatingMode.switchState(OperatingState::State::RunningFromFile);
    request.send(200, "text/html", "start processing file '" + path + "'");
}


void WebServerHooks::setup(Resources &r)
{
    Serial.println("WebServerHooks::setup -> bind webserver hooks");
    r.webServer.reset();

    r.webServer.on("/", [](AsyncWebServerRequest *request) { indexHtml(*request); });
    r.webServer.on("/reboot", [](AsyncWebServerRequest *request) { reboot(*request); });
    r.webServer.on("/status", [](AsyncWebServerRequest *request) { deviceStatus(*request); });
    r.webServer.on("/resetWifi", [&](AsyncWebServerRequest *request)
                   { resetWifi(*request, r.webServer, r.dnsServer); });
    r.webServer.on("/sendGcode", [&](AsyncWebServerRequest *request)
                   { sendGcode(*request, r.gcodeBuffer, r.operatingMode); });
    r.webServer.on("/files", [&](AsyncWebServerRequest *request) { listFiles(*request); });
    r.webServer.on("/file", [&](AsyncWebServerRequest *request) { readFile(*request); });
    r.webServer.on("/runFile", [&](AsyncWebServerRequest *request)
                   { runFile(*request, r.gcodeFileRunner, r.operatingMode); });

    r.webServer.onNotFound([](AsyncWebServerRequest *request)
                           { request->send(404, "text/html", "URL not found."); });
}
