#include "WebServerHooks.h"
#include "FS.h"
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <WString.h>

void indexHtml(AsyncWebServerRequest &request, const String &extra_pre_html = "", const String &extra_post_html = "")
{
    Serial.println("WebServerHooks -> indexHtml");
    String html{ "<!DOCTYPE html>\n"
                 "<html>\n"
                 "  <head>\n"
                 "    <meta http-equiv='Content-Type' content='text/html; charset=utf-8' />\n"
                 "    <title>G-Code Feeder</title>\n"
                 "  </head>\n"
                 "  <body>\n"
                 "    <b>Api Examples</b>\n"
                 "    <p/>\n"
                 "    <table>\n"
                 "      <thead>\n"
                 "        <tr>\n"
                 "          <th align='left'>URL Example</th>\n"
                 "          <th align='left'>Description</th>\n"
                 "          <th align='left'>Arguments</th>\n"
                 "        </tr>\n"
                 "      </thead>\n"
                 "      <tbody id='apiExamples'>\n"
                 "      </tbody>\n"
                 "    </table>\n"
                 "    <p/>\n"
                 "    <b>Manual Move</b>\n"
                 "    <table> \n"
                 "      <tbody>\n"
                 "        <tr> \n"
                 "          <td></td>\n"
                 "          <td><button type='button' onclick='moveZ(true)'>^</button></td>\n"
                 "          <td></td>\n"
                 "        </tr>\n"
                 "        <tr> \n"
                 "          <td><button type='button' onclick='moveX(false)'>&lt;</button></td>\n"
                 "          <td></td>\n"
                 "          <td><button type='button' onclick='moveX(true)'>&gt;</button></td>\n"
                 "        </tr>\n"
                 "          <tr> \n"
                 "          <td></td>\n"
                 "          <td><button type='button' onclick='moveZ(false)'>v</button></td>\n"
                 "          <td></td>\n"
                 "        </tr>\n"
                 "      </tbody>\n"
                 "    </table>"
                 "    <input type='number' id='distance_mm' step='0.1' value='15.0' size='4'></input> "
                 "mm move distance\n"
                 "    <br/>\n"
                 "    <button type=\"button\" onclick=\"setRelativeMotion()\">Relative (send G91) </button>\n"
                 "    <button type=\"button\" onclick=\"setAbsoluteMotion()\">Absolute (send G90)</button>\n"
                 "    <p/>\n"
                 "    <b>File Upload</b>"
                 "    <form method=\"POST\" action=\"/uploadfile\" enctype=\"multipart/form-data\">\n"
                 "      <input type=\"file\" name=\"data\"/>\n"
                 "      <input type=\"submit\" name=\"upload\" value=\"Upload File\" title=\"Upload File\">\n"
                 "    </form>\n"
                 "    <p/>\n"
                 "    <b>Firmware Info</b>\n"
                 "    <ul>\n"
                 "      <li id='sketchHash'>x</li>\n"
                 "      <li id='sketchSize'>x</li>\n"
                 "    </ul>\n"
                 "\n"
                 "    <script>\n"
                 "      fetch('/status')\n"
                 "        .then(response => response.json())\n"
                 "        .then(data => {\n"
                 "          document.querySelector('#sketchHash').innerText = 'hash ' +  data.message.sketch_md5\n"
                 "          document.querySelector('#sketchSize').innerText = 'size ' + data.message.sketch_size_byte\n"
                 "        })\n"
                 "\n"
                 "      fetch('/help')\n"
                 "        .then(response => response.json())\n"
                 "        .then(data => {\n"
                 "          tbody = document.querySelector('#apiExamples')\n"
                 "          for (var example in data.message) {\n"
                 "            var url = example\n"
                 "            var description = data.message[example].description\n"
                 "            var args = data.message[example].args\n"
                 "            var argExample = data.message[example].example\n"
                 "            argExample = argExample ? ('?' + argExample) : ''\n"
                 "            trow = tbody.insertRow()\n"
                 "            a = document.createElement('a')\n"
                 "            a.appendChild(document.createTextNode(url + argExample))\n"
                 "            a.title = url + argExample\n"
                 "            a.href = url + argExample\n"
                 "            trow.insertCell().appendChild(a)\n"
                 "            trow.insertCell().appendChild(document.createTextNode(description))\n"
                 "            trow.insertCell().appendChild(document.createTextNode('[' + args + ']'))\n"
                 "          }\n"
                 "        })\n"
                 "\n"
                 "      function getDistance() { return parseFloat(document.getElementById('distance_mm').value) }\n"
                 "\n"
                 "      function moveX(positive_direction) {\n"
                 "        if (positive_direction)\n"
                 "          fetch('/sendgcode?gcode=G1 X' + getDistance())\n"
                 "        else\n"
                 "          fetch('/sendgcode?gcode=G1 X-' + getDistance())\n"
                 "      }\n"
                 "\n"
                 "      function moveZ(positive_direction) {\n"
                 "        if (positive_direction)\n"
                 "          fetch('/sendgcode?gcode=G1 Z' + getDistance())\n"
                 "        else\n"
                 "          fetch('/sendgcode?gcode=G1 Z-' + getDistance())\n"
                 "      }\n"
                 "\n"
                 "      function setRelativeMotion() { fetch('/sendgcode?gcode=G91') }\n"
                 "      function setAbsoluteMotion() { fetch('/sendgcode?gcode=G90') }\n"
                 "    </script>\n"
                 "  </body>\n"
                 "</html>" };
    request.send(200, "text/html", html);
}


void reboot(AsyncWebServerRequest &request, OperatingState &operatingMode)
{
    Serial.println("WebServerHooks -> reboot: " + request.client()->remoteIP().toString() + " -> " + request.url());
    String info{ "{\n  \"message\":\"reboot\",\n" };
    info += "  \"request\":\"ok\"\n}";
    request.send(200, "application/json", info);
    operatingMode.switchState(OperatingState::State::DoReboot);
}


void deviceStatus(AsyncWebServerRequest &request)
{
    Serial.println("WebServerHooks -> deviceStatus: " + request.client()->remoteIP().toString() + " -> " + request.url());
    String status;
    status += "\n    \"shutdown_reason\":\"";
    status += EspClass::getResetReason().c_str();
    status += "\",\n    \"vcc_mv\":\"";
    status += EspClass::getVcc();
    status += "\",\n    \"free_heap_bytes\":\"";
    status += EspClass::getFreeHeap();
    status += "\",\n    \"free_block_size\":\"";
    status += EspClass::getMaxFreeBlockSize();
    status += "\",\n    \"heap_fragmentation_percent\":\"";
    status += EspClass::getHeapFragmentation();
    status += "\",\n    \"free_continuous_stak_byte\":\"";
    status += EspClass::getFreeContStack();
    status += "\",\n    \"sdk_version\":\"";
    status += EspClass::getSdkVersion();
    status += "\",\n    \"core_version\":\"";
    status += EspClass::getCoreVersion();
    status += "\",\n    \"full_version\":\"";
    status += EspClass::getFullVersion();
    status += "\",\n    \"bootloader_version\":\"";
    status += EspClass::getBootVersion();
    status += "\",\n    \"boot_mode\":\"";
    status += EspClass::getBootMode();
    status += "\",\n    \"cpu_frequency_mhz\":\"";
    status += EspClass::getCpuFreqMHz();
    status += "\",\n    \"flash_chip_id\":\"";
    status += EspClass::getFlashChipId();
    status += "\",\n    \"flash_chip_vendor_id\":\"";
    status += EspClass::getFlashChipVendorId();
    status += "\",\n    \"flash_chip_real_size_byte\":\"";
    status += EspClass::getFlashChipRealSize();
    status += "\",\n    \"flash_chip_size_byte\":\"";
    status += EspClass::getFlashChipSize();
    status += "\",\n    \"flash_chip_speed_hz\":\"";
    status += EspClass::getFlashChipSpeed();
    status += "\",\n    \"flash_chip_mode\":\"";
    status += EspClass::getFlashChipMode();
    status += "\",\n    \"flash_chip_size_by_chip_id\":\"";
    status += EspClass::getFlashChipSizeByChipId();
    status += "\",\n    \"sketch_size_byte\":\"";
    status += EspClass::getSketchSize();
    status += "\",\n    \"sketch_md5\":\"";
    status += EspClass::getSketchMD5();
    status += "\",\n    \"free_sketch_space_byte\":\"";
    status += EspClass::getFreeSketchSpace();
    status += "\",\n    \"reset_reason\":\"";
    status += EspClass::getResetReason();
    status += "\",\n    \"reset_info\":\"";
    status += EspClass::getResetInfo();
    status += "\"";

    request.send(200, "application/json", "{\n  \"message\":{\n" + status + "\n  },\n  \"request\":\"ok\"\n}");
}


void resetWifi(AsyncWebServerRequest &request, OperatingState &operatingMode)
{
    Serial.println("WebServerHooks -> resetWifi: " + request.client()->remoteIP().toString() + " -> " + request.url());
    String info{ "{\n  \"message\":\"reset wifi settings and reboot\",\n" };
    info += "  \"request\":\"ok\"\n}";
    request.send(200, "application/json", info);
    operatingMode.switchState(OperatingState::State::DoResetWifi);
}


void sendGcode(AsyncWebServerRequest &request,
               GcodeBuffer &gcodeBuffer,
               OperatingState &operatingMode,
               const String &extra_pre_html = "",
               const String &extra_post_html = "")
{
    Serial.println("WebServerHooks -> sendGcode: " + request.client()->remoteIP().toString() + " -> " + request.url());

    if(!operatingMode.isState(OperatingState::State::Idle))
    {
        Serial.println("cannot accept gcode via http while not in idle (" + operatingMode.toString() + ")");
        String error{ "{" };
        error += "\n  \"message\":\"cannot accept gcode via http while not in idle,"
                 " current mode is " +
                 operatingMode.toString() + "\",";
        error += "\n  \"request\":\"error\"";
        error += "}";
        request.send(200, "application/json", error);
        return;
    }

    String message;
    String requestStatus{ "ok" };
    if(request.hasParam("gcode"))
    {
        if(request.hasArg("gcode"))
        {
            String gcode{ request.getParam("gcode")->value() };
            message += "gcode='" + gcode + "'";

            if(gcodeBuffer.isProcessed())
            {
                Serial.println("buffer gcode='" + gcode + "'");
                message += " buffered";
            }
            else
            {
                Serial.println("failed to buffer gcode '" + gcode + "' while processing other gcode '" +
                               gcodeBuffer.getGcode() + "'");
                message += " not buffered, still processing other gcode '" + gcodeBuffer.getGcode() + "'";
                requestStatus = "error";
            }
        }
        else
        {
            message = "failed to retrieve value of argument 'gcode' from http request";
            requestStatus = "error";
            Serial.println(message);
        }
    }
    else
    {
        message = "failed to retrieve argument 'gcode' from http request";
        requestStatus = "error";
    }

    request.send(200, "application/json", "{\n  \"message\":\"" + message + "\",\n  \"request\":\"" + requestStatus + "\"\n}");
}


void listFiles(AsyncWebServerRequest &request)
{
    Serial.println("WebServerHooks -> listFiles: " + request.client()->remoteIP().toString() + " -> " + request.url());

    String message;
    String requestStatus{ "ok" };

    const String path = "/";
    File root = LittleFS.open(path, "r");
    if(!root)
    {
        message = "failed to open directory";
        Serial.println(message);
        request.send(200, "application/json", "{\n  \"message\":\"" + message + "\",\n  \"request\":\"error\"\n}");
        return;
    }
    if(!root.isDirectory())
    {
        message = "'" + path + "' is not a directory";
        Serial.println(message);
        request.send(200, "application/json", "{\n  \"message\":\"" + message + "\",\n  \"request\":\"error\"\n}");
        return;
    }

    File file = root.openNextFile();
    String isFirstItem{ true };
    while(file)
    {
        String type{ file.isDirectory() ? "dir" : "file" };
        String fileSize{ file.isDirectory() ? "0" : String(file.size()) };
        const time_t t = file.getLastWrite();
        const struct tm *ts = localtime(&t);

        if(isFirstItem)
        {
            isFirstItem = false;
            message += String() + "\n    \"" + file.name() + "\":{\n  ";
        }
        else
            message += String() + ",\n    \"" + file.name() + "\":{\n";

        message += String() + R"(      "type":")" + type + "\",\n";
        message += String() + R"(      "year":")" + String((ts->tm_year) + 1900) + "\",\n";
        message += String() + R"(      "month":")" + String(ts->tm_mon + 1) + "\",\n";
        message += String() + R"(      "day":")" + String(ts->tm_mday) + "\",\n";
        message += String() + R"(      "hour":")" + String(ts->tm_hour) + "\",\n";
        message += String() + R"(      "minute":")" + String(ts->tm_min) + "\",\n";
        message += String() + R"(      "second":")" + String(ts->tm_sec) + "\",\n";
        message += String() + R"(      "size_byte":")" + fileSize + "\"\n";
        message += "    }";

        file = root.openNextFile();
    }

    request.send(200, "application/json", "{\n  \"message\":{" + message + "},\n  \"request\":\"ok\"\n}");
}


void readFile(AsyncWebServerRequest &request)
{
    Serial.println("WebServerHooks -> readFile: " + request.client()->remoteIP().toString() + " -> " + request.url());
    String path = "NA";

    if(request.hasArg("name"))
        path = request.getParam("name")->value();

    if(!LittleFS.exists(path))
    {
        const String message{ "failed to read file '" + path + "', file does not exist" };
        Serial.println(message);
        request.send(200, "application/json", "{\n  \"message\":\"" + message + "\",\n  \"request\":\"error\"\n}");
        return;
    }

    Serial.println("read file '" + path + "'");
    request.send(LittleFS, path, "text/plain");
}


void deleteFile(AsyncWebServerRequest &request)
{
    Serial.println("WebServerHooks -> deleteFile: " + request.client()->remoteIP().toString() + " -> " + request.url());
    String path = "NA";

    if(request.hasArg("name"))
        path = request.getParam("name")->value();

    if(!LittleFS.exists(path))
    {
        const String message{ "failed to delete file '" + path + "', file does not exist" };
        Serial.println(message);
        request.send(200, "application/json", "{\n  \"message\":\"" + message + "\",\n  \"request\":\"error\"\n}");
        return;
    }

    String result;
    String message;
    if(!LittleFS.remove(path))
    {
        result = "error";
        message = "failed to delete file '" + path + "'";
        Serial.println(message);
    }
    else
    {
        result = "ok";
        message = "file '" + path + "' deleted";
        Serial.println(message);
    }
    request.send(200, "application/json", "{\n  \"message\":\"" + message + "\",\n  \"request\":\"" + result + "\"\n}");
}


void runFile(AsyncWebServerRequest &request, GcodeFileRunner &fileRunner, OperatingState &operatingMode)
{
    Serial.println("WebServerHooks -> runFile: " + request.client()->remoteIP().toString() + " -> " + request.url());
    String path = "NA";

    if(!operatingMode.isState(OperatingState::State::Idle))
    {
        const String message{ "cannot start processing file while not in idle (" + operatingMode.toString() + ")" };
        Serial.println(message);
        request.send(200, "application/json", "{\n  \"message\":\"" + message + "\",\n  \"request\":\"error\"\n}");
        return;
    }

    if(request.hasArg("name"))
        path = request.getParam("name")->value();

    if(!LittleFS.exists(path))
    {
        const String message{ "failed to start processing file '" + path + "', file does not exist" };
        Serial.println(message);
        request.send(200, "application/json", "{\n  \"message\":\"" + message + "\",\n  \"request\":\"error\"\n}");
        return;
    }

    fileRunner.reset();
    fileRunner.setFilepath(path);
    operatingMode.switchState(OperatingState::State::RunningFromFile);
    const String message{ "start processing file '" + path + "'" };
    request.send(200, "application/json", "{\n  \"message\":\"" + message + "\",\n  \"request\":\"ok\"\n}");
}


void getOperatingMode(AsyncWebServerRequest &request, OperatingState &operatingMode)
{
    Serial.println("WebServerHooks -> getOperatingMode: " + request.client()->remoteIP().toString() + " -> " + request.url());
    request.send(200, "application/json", "{\n  \"message\":\"" + operatingMode.toString() + "\",\n  \"request\":\"ok\"\n}");
}


void getGcodeStatus(AsyncWebServerRequest &request, GcodeBuffer &gcodeBuffer)
{
    Serial.println("WebServerHooks -> getGcodeStatus: " + request.client()->remoteIP().toString() + " -> " + request.url());
    String info;
    info += String() + R"("gcode":")" + gcodeBuffer.getGcode() + "\",\n    ";
    info += String() + R"("is_transmitted":")" + gcodeBuffer.isTransmitted() + "\",\n    ";
    info += String() + R"("is_processed":")" + gcodeBuffer.isProcessed() + "\",\n    ";
    info += String() + R"("response":")" + gcodeBuffer.getResponse() + "\",\n    ";
    info += String() + R"("is_response_ok":")" + gcodeBuffer.isResponseOk() + "\",\n    ";
    info += String() + R"("response_error_code":")" + gcodeBuffer.getErrorCode() + "\",\n    ";
    info += String() + R"("is_motion_finished":")" + gcodeBuffer.isMotionFinished() + "\"  ";

    request.send(200, "application/json", "{\n  \"message\":{\n    " + info + "},\n  \"request\":\"ok\"\n}");
}


void handleUpload(AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final)
{
    Serial.println("WebServerHooks -> handleUplod: " + request->client()->remoteIP().toString() + " -> " + request->url());

    if(!index)
    {
        Serial.println("upload '" + String(filename) + "' started");
        request->_tempFile = LittleFS.open("/" + filename, "w");
    }

    if(len)
    {
        request->_tempFile.write(data, len);
        Serial.println("writing file '" + String(filename) + "' index=" + String(index) + " len=" + String(len));
    }

    if(final)
    {
        request->_tempFile.close();
        Serial.println("upload '" + String(filename) + "' completed, size=" + String(index + len));
        request->redirect("/");
    }
}


void WebServerHooks::setup(Resources &r)
{
    Serial.println("WebServerHooks::setup -> bind webserver hooks");
    r.webServer.reset();

    r.webServer.on("/", [](AsyncWebServerRequest *request) { indexHtml(*request); });
    r.webServer.on("/reboot", [&](AsyncWebServerRequest *request) { reboot(*request, r.operatingMode); });
    r.webServer.on("/status", [](AsyncWebServerRequest *request) { deviceStatus(*request); });
    r.webServer.on("/resetwifi", [&](AsyncWebServerRequest *request) { resetWifi(*request, r.operatingMode); });

    r.webServer.on("/files", [&](AsyncWebServerRequest *request) { listFiles(*request); });
    r.webServer.on("/file", [&](AsyncWebServerRequest *request) { readFile(*request); });
    r.webServer.on("/deletefile", [&](AsyncWebServerRequest *request) { deleteFile(*request); });
    r.webServer.on(
    "/uploadfile", HTTP_POST, [](AsyncWebServerRequest *request) { request->send(200); }, handleUpload);

    r.webServer.on("/sendgcode",
                   [&](AsyncWebServerRequest *request) { sendGcode(*request, r.gcodeBuffer, r.operatingMode); });
    r.webServer.on("/runfile",
                   [&](AsyncWebServerRequest *request) { runFile(*request, r.gcodeFileRunner, r.operatingMode); });
    r.webServer.on("/operatingmode", [&](AsyncWebServerRequest *request) { getOperatingMode(*request, r.operatingMode); });

    r.webServer.on("/gcodestatus", [&](AsyncWebServerRequest *request) { getGcodeStatus(*request, r.gcodeBuffer); });

    r.webServer.on(
    "/help",
    [](AsyncWebServerRequest *request)
    {
        // clang-format off
                       const String knownHooks{
                           "\n" R"(    "/":{"args":[], "example":"", "description":"welcome page"},)"
                           "\n" R"(    "/status":{"args":[], "example":"", "description":"retrieve hardware status"},)"
                           "\n" R"(    "/operatingmode":{"args":[], "example":"", "description":"retrieve the current operating state, see OperatingState.h"},)"
                           "\n" R"(    "/reboot":{"args":[], "example":"", "description":"reboot hardware"},)"
                           "\n" R"(    "/resetwifi":{"args":[], "example":"", "description":"reset wifi settings and reboot"},)"
                           "\n" R"(    "/sendgcode":{"args":["gcode"], "example":"gcode=G91", "description":"send <gcode> to controller"},)"
                           "\n" R"(    "/gcodestatus":{"args":[], "example":"", "description":"retrieve the processing status of last gcode"},)"
                           "\n" R"(    "/file":{"args":["name"], "example":"name=test.g", "description":"retrieve the content of file <name> as text/plain, in case of error json as application/json"},)"
                           "\n" R"(    "/deletefile":{"args":["name"], "example":"name=test.g", "description":"delete a file from filesystem"},)"
                           "\n" R"(    "/uploadfile":{"args":[""], "example":"", "description":"upload a file to filesystem, method POST"},)"
                           "\n" R"(    "/files":{"args":[], "example":"", "description":"lists content of the top level directory"},)"
                           "\n" R"(    "/runfile":{"args":["name"], "example":"name=test.g", "description":"runs gcode from file <name>"},)"
                           "\n" R"(    "/help":{"args":[], "example":"", "description":"retrieves the api description"})"
                       };
        // clang-format on
        request->send(200, "application/json", "{\n  \"message\":{ " + knownHooks + "\n  },\n  \"request\":\"ok\"\n}");
    });

    r.webServer.onNotFound(
    [](AsyncWebServerRequest *request)
    { request->send(404, "application/json", "{\n  \"message\":\"URL not found\",\n  \"request\":\"error\"\n}"); });
}
