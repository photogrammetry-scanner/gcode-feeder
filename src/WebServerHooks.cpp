#if !defined(ENV_NATIVE)
#include "WebServerHooks.h"
#include "FS.h"
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <string>

void indexHtml(AsyncWebServerRequest &request, const String &extra_pre_html = "", const String &extra_post_html = "")
{
    Serial.println(std::string(std::to_string(millis()) + " WebServerHooks -> indexHtml").c_str());
    std::string html{
        "<!DOCTYPE html>\n"
        "<html>\n"
        "  <head>\n"
        "    <meta http-equiv='Content-Type' content='text/html; charset=utf-8' />\n"
        "    <title>G-Code Feeder</title>\n"
        "    <link rel='icon' href='data:,'>\n"
        "  </head>\n"
        "  <body>\n"
        "    <b>Api Description</b>\n"
        "    <p/>\n"
        "    <table>\n"
        "      <thead>\n"
        "        <tr>\n"
        "          <th align='left'>URL Example</th>\n"
        "          <th align='left'>Description</th>\n"
        "          <th align='left'>Arguments</th>\n"
        "          <th align='left'>Argument Examples</th>\n"
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
        "          <td><button type='button' onclick='moveZ(true)'>&ShortUpArrow;</button></td>\n"
        "          <td></td>\n"
        "        </tr>\n"
        "        <tr> \n"
        "          <td><button type='button' onclick='moveX(false)'>&ShortLeftArrow;</button></td>\n"
        "          <td><button type='button' onclick='triggerHoming()'>&#x2302;</button></td>\n"
        "          <td><button type='button' onclick='moveX(true)'>&rightarrow;</button></td>\n"
        "        </tr>\n"
        "          <tr> \n"
        "          <td></td>\n"
        "          <td><button type='button' onclick='moveZ(false)'>&ShortDownArrow;</button></td>\n"
        "          <td></td>\n"
        "        </tr>\n"
        "      </tbody>\n"
        "    </table>"
        "    <input type='number' id='distance_mm' step='0.1' value='15.0' size='4'></input> "
        "mm move distance<br/>\n"
        "    <input type='number' id='feed_rate_mm_min' step='10' value='200' min='10' max='500' size='4'></input> "
        "mm/min feed rate<br/>\n"
        "    <input type='number' id='acceleration_mm_s2' step='50' value='200' min='10' max='1000' size='4'></input> "
        "mm/s², set acceleration: \n"
        "    <button type=\"button\" onclick=\"setAcceleration('X')\">X</button>\n"
        "    <button type=\"button\" onclick=\"setAcceleration('Y')\">Y</button>\n"
        "    <button type=\"button\" onclick=\"setAcceleration('Z')\">Z</button>\n"
        "    <br/>\n"
        "    Distance mode: \n"
        "    <button type=\"button\" onclick=\"setRelativeMotion()\">Incremental (G91) </button>\n"
        "    <button type=\"button\" onclick=\"setAbsoluteMotion()\">Absolute (G90)</button>\n"
        "    <p/>\n"
        "    <b>File Upload</b>"
        "    <form method=\"POST\" action=\"/api/uploadfile\" enctype=\"multipart/form-data\">\n"
        "      <input type=\"file\" name=\"data\"/>\n"
        "      <input type=\"submit\" name=\"upload\" value=\"Upload File\" title=\"Upload File\">\n"
        "    </form>\n"
        "    <p/>\n"
        "    <b>Firmware Info</b>\n"
        "    <ul>\n"
        "      <li id='sketchHash'>x</li>\n"
        "      <li id='sketchSize'>x</li>\n"
        "    </ul>\n"
        "    <hr/>\n"
        "    For more supported G-codes refer to this <a "
        "href='https://github.com/photogrammetry-scanner/docs/blob/main/README-GRBL.md'>documentation</a>.\n"
        "\n"
        "    <script>\n"
        "      fetch('/api/esp/state')\n"
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
        "          for (var key in data.message) {\n"
        "            var item = data.message[key]\n"
        "            var url = key\n"
        "            var description = item.description\n"
        "            var args = item.args\n"
        "            var argsExamples = item.args_examples\n"
        "            var apiExample = (argsExamples.length > 0) ? (url + '?' + argsExamples[0]) : url\n"
        "            var trow = tbody.insertRow()\n"
        "            var a = document.createElement('a')\n"
        "            a.appendChild(document.createTextNode(apiExample))\n"
        "            a.title = apiExample\n"
        "            a.href = apiExample\n"
        "            trow.insertCell().appendChild(a)\n"
        "            trow.insertCell().appendChild(document.createTextNode(description))\n"
        "            trow.insertCell().appendChild(document.createTextNode('[' + args + ']'))\n"
        "            trow.insertCell().appendChild(document.createTextNode('[' + argsExamples + ']'))\n"
        "          }\n"
        "        })\n"
        "\n"
        "      function getFeedRate() { return parseInt(document.getElementById('feed_rate_mm_min').value) }\n"
        "      function getDistance() { return parseFloat(document.getElementById('distance_mm').value) }\n"
        "      function getAcceleration() { return parseInt(document.getElementById('acceleration_mm_s2').value) }\n"
        "\n"
        "      function moveX(positive_direction) {\n"
        "        var d = (positive_direction) ? getDistance() : -getDistance()\n"
        "        var f = getFeedRate()\n"
        "        fetch('/api/gcode/send?gcode=G1 F' + f + ' X' + d)\n"
        "      }\n"
        "\n"
        "      function moveZ(positive_direction) {\n"
        "        var d = (positive_direction) ? getDistance() : -getDistance()\n"
        "        var f = getFeedRate()\n"
        "        fetch('/api/gcode/send?gcode=G1 F' + f + ' Z' + d)\n"
        "      }\n"
        "\n"
        "      function setAcceleration(axis) {\n"
        "        var ax='$120'\n"
        "        if (axis == 'Y') \n"
        "          ax='$121'\n"
        "        if (axis == 'Z') \n"
        "          ax='$122'\n"
        "        var acc = getAcceleration()\n"
        "        fetch('/api/gcode/send?gcode=' + ax + '=' + acc)\n"
        "      }\n"
        "\n"
        "      function triggerHoming() { fetch('/api/gcode/send?gcode=$H') }\n"
        "      function setRelativeMotion() { fetch('/api/gcode/send?gcode=G91') }\n"
        "      function setAbsoluteMotion() { fetch('/api/gcode/send?gcode=G90') }\n"
        "    </script>\n"
        "  </body>\n"
        "</html>"
    };
    request.send(200, "text/html", html.c_str());
}


void reboot(AsyncWebServerRequest &request, OperatingState &operatingMode)
{
    Serial.println(std::string(std::to_string(millis()) + " WebServerHooks::reboot: " +
                               request.client()->remoteIP().toString().c_str() + " -> " + request.url().c_str())
                   .c_str());
    std::string info{ "{\n  \"message\":\"reboot\",\n" };
    info += "  \"request\":\"ok\"\n}";
    request.send(200, "application/json", info.c_str());
    operatingMode.switchState(OperatingState::State::DoReboot);
}


void deviceStatus(AsyncWebServerRequest &request)
{
    Serial.println(std::string(std::to_string(millis()) + " WebServerHooks::deviceStatus: " +
                               request.client()->remoteIP().toString().c_str() + " -> " + request.url().c_str())
                   .c_str());
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
    Serial.println(std::string(std::to_string(millis()) + " WebServerHooks::resetWifi: " +
                               request.client()->remoteIP().toString().c_str() + " -> " + request.url().c_str())
                   .c_str());
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
    Serial.println(std::string(std::to_string(millis()) + " WebServerHooks::sendGcode: " +
                               request.client()->remoteIP().toString().c_str() + " -> " + request.url().c_str())
                   .c_str());

    if(!operatingMode.isState(OperatingState::State::Idle))
    {
        Serial.println(std::string(std::to_string(millis()) + " cannot accept gcode via http while not in idle (" +
                                   operatingMode.toString() + ")")
                       .c_str());
        std::string error{ "{" };
        error += "\n  \"message\":\"cannot accept gcode via http while not in idle,"
                 " current mode is " +
                 operatingMode.toString() + "\",";
        error += "\n  \"request\":\"error\"";
        error += "}";
        request.send(200, "application/json", error.c_str());
        return;
    }

    std::string message;
    std::string requestStatus{ "ok" };
    if(request.hasParam("gcode"))
    {
        if(request.hasArg("gcode"))
        {
            std::string gcode{ request.getParam("gcode")->value().c_str() };
            message += std::string("gcode='" + gcode + "'");

            if(gcodeBuffer.isProcessed())
            {
                Serial.println(std::string(std::to_string(millis()) + " buffer gcode='" + gcode + "'").c_str());
                message += " buffered";
                gcodeBuffer.setGcode(gcode);
            }
            else
            {
                Serial.println(std::string(std::to_string(millis()) + " failed to buffer gcode '" + gcode +
                                           "' while processing other gcode '" + gcodeBuffer.getGcode() + "'")
                               .c_str());
                message += " not buffered, still processing other gcode '" + gcodeBuffer.getGcode() + "'";
                requestStatus = "error";
            }
        }
        else
        {
            message = "failed to retrieve value of argument 'gcode' from http request";
            requestStatus = "error";
            Serial.println(std::string(std::to_string(millis()) + " " + message).c_str());
        }
    }
    else
    {
        message = "failed to retrieve argument 'gcode' from http request";
        requestStatus = "error";
    }

    request.send(200, "application/json",
                 std::string("{\n  \"message\":\"" + message + "\",\n  \"request\":\"" + requestStatus + "\"\n}").c_str());
}


void listFiles(AsyncWebServerRequest &request)
{
    Serial.println(std::string(std::to_string(millis()) + " WebServerHooks::listFiles: " +
                               request.client()->remoteIP().toString().c_str() + " -> " + request.url().c_str())
                   .c_str());

    std::string message;

    const std::string path = "/";
    File root = LittleFS.open(path.c_str(), "r");
    if(!root)
    {
        message = "failed to open directory";
        Serial.println(std::string(std::to_string(millis()) + " " + message).c_str());
        request.send(200, "application/json",
                     std::string("{\n  \"message\":\"" + message + "\",\n  \"request\":\"error\"\n}").c_str());
        return;
    }
    if(!root.isDirectory())
    {
        message = "'" + path + "' is not a directory";
        Serial.println(std::string(std::to_string(millis()) + " " + message).c_str());
        request.send(200, "application/json",
                     std::string("{\n  \"message\":\"" + message + "\",\n  \"request\":\"error\"\n}").c_str());
        return;
    }

    File file = root.openNextFile();
    bool isFirstItem{ true };
    while(file)
    {
        std::string type{ file.isDirectory() ? "dir" : "file" };
        std::string fileSize{ file.isDirectory() ? "0" : std::to_string(file.size()) };
        const time_t t = file.getLastWrite();
        const struct tm *ts = localtime(&t);

        if(isFirstItem)
        {
            isFirstItem = false;
            message += "\n    \"" + std::string(file.name()) + "\":{\n  ";
        }
        else
            message += ",\n    \"" + std::string(file.name()) + "\":{\n";

        message += R"(      "type":")" + type + "\",\n";
        message += R"(      "year":")" + std::to_string((ts->tm_year) + 1900) + "\",\n";
        message += R"(      "month":")" + std::to_string(ts->tm_mon + 1) + "\",\n";
        message += R"(      "day":")" + std::to_string(ts->tm_mday) + "\",\n";
        message += R"(      "hour":")" + std::to_string(ts->tm_hour) + "\",\n";
        message += R"(      "minute":")" + std::to_string(ts->tm_min) + "\",\n";
        message += R"(      "second":")" + std::to_string(ts->tm_sec) + "\",\n";
        message += R"(      "size_byte":")" + fileSize + "\"\n";
        message += "    }";

        file = root.openNextFile();

        if(file)
            message += ',';
    }

    request.send(200, "application/json", std::string("{\n  \"message\":{" + message + "},\n  \"request\":\"ok\"\n}").c_str());
}


void readFile(AsyncWebServerRequest &request)
{
    Serial.println(std::string(std::to_string(millis()) + " WebServerHooks::readFile: " +
                               request.client()->remoteIP().toString().c_str() + " -> " + request.url().c_str())
                   .c_str());
    std::string path = "NA";

    if(request.hasArg("name"))
        path = request.getParam("name")->value().c_str();

    if(!LittleFS.exists(path.c_str()))
    {
        const std::string message{ "failed to read file '" + path + "', file does not exist" };
        Serial.println(std::string(std::to_string(millis()) + " " + message).c_str());
        request.send(200, "application/json",
                     std::string("{\n  \"message\":\"" + message + "\",\n  \"request\":\"error\"\n}").c_str());
        return;
    }

    Serial.println(std::string(std::to_string(millis()) + " read file '" + path + "'").c_str());
    request.send(LittleFS, path.c_str(), "text/plain");
}


void deleteFile(AsyncWebServerRequest &request)
{
    Serial.println(std::string(std::to_string(millis()) + " WebServerHooks::deleteFile: " +
                               request.client()->remoteIP().toString().c_str() + " -> " + request.url().c_str())
                   .c_str());
    std::string path = "NA";

    if(request.hasArg("name"))
        path = request.getParam("name")->value().c_str();

    if(!LittleFS.exists(path.c_str()))
    {
        const std::string message{ "failed to delete file '" + path + "', file does not exist" };
        Serial.println(std::string(std::to_string(millis()) + " " + message).c_str());
        request.send(200, "application/json",
                     std::string("{\n  \"message\":\"" + message + "\",\n  \"request\":\"error\"\n}").c_str());
        return;
    }

    std::string result;
    std::string message;
    if(!LittleFS.remove(path.c_str()))
    {
        result = "error";
        message = "failed to delete file '" + path + "'";
        Serial.println(std::string(std::to_string(millis()) + " " + message).c_str());
    }
    else
    {
        result = "ok";
        message = "file '" + path + "' deleted";
        Serial.println(std::string(std::to_string(millis()) + " " + message).c_str());
    }
    request.send(200, "application/json",
                 std::string("{\n  \"message\":\"" + message + "\",\n  \"request\":\"" + result + "\"\n}").c_str());
}


void runFile(AsyncWebServerRequest &request, GcodeFileRunner &fileRunner, OperatingState &operatingMode)
{
    Serial.println(std::string(std::to_string(millis()) + " WebServerHooks::runFile: " +
                               request.client()->remoteIP().toString().c_str() + " -> " + request.url().c_str())
                   .c_str());
    std::string path = "NA";

    if(!operatingMode.isState(OperatingState::State::Idle))
    {
        const std::string message{ "cannot start processing file while not in idle (" + operatingMode.toString() + ")" };
        Serial.println(std::string(std::to_string(millis()) + " " + message).c_str());
        request.send(200, "application/json",
                     std::string("{\n  \"message\":\"" + message + "\",\n  \"request\":\"error\"\n}").c_str());
        return;
    }

    if(request.hasArg("name"))
        path = request.getParam("name")->value().c_str();

    if(!LittleFS.exists(path.c_str()))
    {
        const std::string message{ "failed to start processing file '" + path + "', file does not exist" };
        Serial.println(std::string(std::to_string(millis()) + " " + message).c_str());
        request.send(200, "application/json",
                     std::string("{\n  \"message\":\"" + message + "\",\n  \"request\":\"error\"\n}").c_str());
        return;
    }

    fileRunner.reset();
    fileRunner.setFilepath(path);
    operatingMode.switchState(OperatingState::State::RunningFromFile);
    const std::string message{ "start processing file '" + path + "'" };
    request.send(200, "application/json", std::string("{\n  \"message\":\"" + message + "\",\n  \"request\":\"ok\"\n}").c_str());
}


void getOperatingMode(AsyncWebServerRequest &request, OperatingState &operatingMode)
{
    Serial.println(std::string(std::to_string(millis()) + " WebServerHooks::getOperatingMode: " +
                               request.client()->remoteIP().toString().c_str() + " -> " + request.url().c_str())
                   .c_str());
    request.send(200, "application/json",
                 std::string("{\n  \"message\":\"" + operatingMode.toString() + "\",\n  \"request\":\"ok\"\n}").c_str());
}


void getGcodeStatus(AsyncWebServerRequest &request, const GcodeBuffer &gcodeBuffer)
{
    Serial.println(std::string(std::to_string(millis()) + " WebServerHooks::getGcodeStatus: " +
                               request.client()->remoteIP().toString().c_str() + " -> " + request.url().c_str())
                   .c_str());
    std::string info;
    info += std::string(+R"("gcode":")" + gcodeBuffer.getGcode() + "\",\n    ");
    info += std::string(+R"("is_transmitted":")" + std::string(gcodeBuffer.isTransmitted() ? "true" : "false") + "\",\n    ");
    info += std::string(+R"("is_processed":")" + std::string(gcodeBuffer.isProcessed() ? "true" : "false") + "\",\n    ");
    info += std::string(+R"("response_string":")" + gcodeBuffer.getResponse() + "\",\n    ");
    info += std::string(+R"("is_response_ok":")" + std::string(gcodeBuffer.isResponseOk() ? "true" : "false") + "\",\n    ");
    info += std::string(+R"("response_error_code":")" + std::to_string(gcodeBuffer.getErrorCode()) + "\",\n    ");
    info += std::string(+R"("is_motion_finished":")" + std::string(gcodeBuffer.isMotionFinished() ? "true" : "false") + "\"  ");

    request.send(200, "application/json", std::string("{\n  \"message\":{\n    " + info + "},\n  \"request\":\"ok\"\n}").c_str());
}


void handleUpload(AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final)
{
    Serial.println(std::string(std::to_string(millis()) + " WebServerHooks::handleUpload: " +
                               request->client()->remoteIP().toString().c_str() + " -> " + request->url().c_str())
                   .c_str());

    if(!index)
    {
        Serial.println(std::string(std::to_string(millis()) + " upload '" + std::string(filename.c_str()) + "' started").c_str());
        request->_tempFile = LittleFS.open("/" + filename, "w");
    }

    if(len)
    {
        request->_tempFile.write(data, len);
        Serial.println(std::string(std::to_string(millis()) + " writing file '" + std::string(filename.c_str()) +
                                   "' index=" + std::to_string(index) + " len=" + std::to_string(len))
                       .c_str());
    }

    if(final)
    {
        request->_tempFile.close();
        Serial.println(std::string(std::to_string(millis()) + " upload '" + std::string(filename.c_str()) +
                                   "' completed, size=" + std::to_string(index + len))
                       .c_str());
        request->redirect("/");
    }
}


void WebServerHooks::setup(Resources &r)
{
    Serial.println(std::string(std::to_string(millis()) + " WebServerHooks::setup -> bind webserver hooks").c_str());
    r.webServer.reset();

    r.webServer.on("/", [](AsyncWebServerRequest *request) { indexHtml(*request); });
    r.webServer.on("/api/esp/reboot", [&](AsyncWebServerRequest *request) { reboot(*request, r.operatingMode); });
    r.webServer.on("/api/esp/state", [](AsyncWebServerRequest *request) { deviceStatus(*request); });
    r.webServer.on("/api/esp/resetwifi", [&](AsyncWebServerRequest *request) { resetWifi(*request, r.operatingMode); });

    r.webServer.on("/api/file/ls", [&](AsyncWebServerRequest *request) { listFiles(*request); });
    r.webServer.on("/api/file/cat", [&](AsyncWebServerRequest *request) { readFile(*request); });
    r.webServer.on("/api/file/delete", [&](AsyncWebServerRequest *request) { deleteFile(*request); });
    r.webServer.on(
    "/api/file/upload", HTTP_POST, [](AsyncWebServerRequest *request) { request->send(200); }, handleUpload);

    r.webServer.on("/api/gcode/send",
                   [&](AsyncWebServerRequest *request) { sendGcode(*request, r.gcodeBuffer, r.operatingMode); });
    r.webServer.on("/api/file/run",
                   [&](AsyncWebServerRequest *request) { runFile(*request, r.gcodeFileRunner, r.operatingMode); });
    r.webServer.on("/api/firmware/state",
                   [&](AsyncWebServerRequest *request) { getOperatingMode(*request, r.operatingMode); });

    r.webServer.on("/api/gcode/status", [&](AsyncWebServerRequest *request) { getGcodeStatus(*request, r.gcodeBuffer); });

    r.webServer.on(
    "/help",
    [](AsyncWebServerRequest *request)
    {
        // clang-format off
        // note: the first argument of args_example is taken for the example-url
        const String knownHooks{
            "\n" R"(    "/":{"args":[], "args_examples":[], "description":"welcome page"},)"
            "\n" R"(    "/help":{"args":[], "args_examples":[], "description":"retrieves the api description"},)"
            "\n" R"(    "/api/esp/state":{"args":[], "args_examples":[], "description":"retrieve hardware status"},)"
            "\n" R"(    "/api/esp/reboot":{"args":[], "args_examples":[], "description":"reboot hardware"},)"
            "\n" R"(    "/api/esp/resetwifi":{"args":[], "args_examples":[], "description":"reset wifi settings and reboot"},)"
            "\n" R"(    "/api/gcode/send":{"args":["gcode"], "args_examples":["gcode=G91", "gcode=G90", "gcode=G1 X10","gcode=$H"], "description":"send <gcode> to controller"},)"
            "\n" R"(    "/api/gcode/status":{"args":[], "args_examples":[], "description":"retrieve the processing status of last gcode"},)"
            "\n" R"(    "/api/file/run":{"args":["name"], "args_examples":["name=test.g", "name=test.gcode"], "description":"runs gcode from file <name>"},)"
            "\n" R"(    "/api/file/cat":{"args":["name"], "args_examples":["name=test.g", "name=foo.txt"], "description":"retrieve the content of file <name> as text/plain, in case of error json as application/json"},)"
            "\n" R"(    "/api/file/ls":{"args":[], "args_examples":[], "description":"lists content of the top level directory"},)"
            "\n" R"(    "/api/file/upload":{"args":[], "args_examples":[], "description":"upload a file to filesystem, method POST"},)"
            "\n" R"(    "/api/file/delete":{"args":["name"], "args_examples":["name=test.g", "name=foo.txt"], "description":"delete a file from filesystem"},)"
            "\n" R"(    "/api/firmware/state":{"args":[], "args_examples":[], "description":"retrieve the current operating state, see OperatingState.h"})"
        };
        // clang-format on
        request->send(200, "application/json", "{\n  \"message\":{ " + knownHooks + "\n  },\n  \"request\":\"ok\"\n}");
    });

    r.webServer.onNotFound(
    [](AsyncWebServerRequest *request)
    { request->send(404, "application/json", "{\n  \"message\":\"URL not found\",\n  \"request\":\"error\"\n}"); });
}

#endif
