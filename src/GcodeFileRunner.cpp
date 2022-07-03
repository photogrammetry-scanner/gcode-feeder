#if !defined(ENV_NATIVE)
#include "GcodeFileRunner.h"
#include "GcodeBuffer.h"
#include "OperatingState.h"
#include <LittleFS.h>


GcodeFileRunner::GcodeFileRunner(GcodeBuffer &buffer, OperatingState &opMode)
: filePath("test.g"), currentLine(0), gcodeBuffer(buffer), operatingMode(opMode)
{
}


void GcodeFileRunner::process()
{
    if(file && !operatingMode.isState(OperatingState::State::RunningFromFile))
    {
        Serial.println(std::string("close file '" + std::string(file.name()) + "'").c_str());
        currentLine = 0;
        file.close();
    }

    if(!file)
    {
        Serial.println(std::string("gcode file '" + filePath + "' not opened").c_str());
        file = LittleFS.open(filePath.c_str(), "r");
        if(file.isDirectory())
        {
            Serial.println(std::string("file '" + std::string(file.name()) + "' is a directory, closing file").c_str());
            file.close();
        }
        else
        {
            Serial.println(std::string("gcode file '" + std::string(file.name()) + "' opened for processing").c_str());
        }
    }

    if(gcodeBuffer.isProcessed())
    {
        if(file.available())
        {
            gcodeBuffer.setGcode(file.readStringUntil('\n').c_str());
            currentLine++;
            Serial.println(
            std::string("processing line " + std::to_string(currentLine) + ": '" + gcodeBuffer.getGcode() + "'").c_str());
        }
        else
        {
            Serial.println(std::string("file '" + std::string(file.name()) + "' processed").c_str());
            operatingMode.switchState(OperatingState::State::FinishedFromFile);
            Serial.println(std::string("close file '" + std::string(file.name()) + "'").c_str());
            currentLine = 0;
            file.close();
            operatingMode.switchState(OperatingState::State::Idle);
        }
    }
}


void GcodeFileRunner::reset()
{
    file.close();
    filePath = "";
    currentLine = 0;
}


bool GcodeFileRunner::setFilepath(const std::string &path)
{
    if(file)
    {
        Serial.println("failed to set new gcode file, try reset() first");
        return false;
    }

    filePath = path;
    return true;
}


const uint32_t &GcodeFileRunner::getCurrentLine() const { return currentLine; }

#endif
