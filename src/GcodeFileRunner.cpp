#if !defined(ENV_NATIVE)
#include "GcodeFileRunner.h"
#include "GcodeBuffer.h"
#include "OperatingState.h"
#include <LittleFS.h>


GcodeFileRunner::GcodeFileRunner(GcodeBuffer &buffer, OperatingState &opMode)
: filePath("test.g"), currentLine(0), gcodeBuffer(buffer), operatingMode(opMode)
{
}


void GcodeFileRunner::tryBufferNextLine()
{
    if(file && !operatingMode.isState(OperatingState::State::RunningFromFile))
    {
        Serial.println(std::string(std::to_string(millis()) + " close file '" + file.name() + "'").c_str());
        currentLine = 0;
        file.close();
    }

    if(!file)
    {
        Serial.println(std::string(std::to_string(millis()) + " gcode file '" + filePath + "' not opened").c_str());
        file = LittleFS.open(filePath.c_str(), "r");
        if(file.isDirectory())
        {
            Serial.println(
            std::string(std::to_string(millis()) + " file '" + file.name() + "' is a directory, closing file").c_str());
            file.close();
        }
        else
        {
            Serial.println(
            std::string(std::to_string(millis()) + " gcode file '" + file.name() + "' opened for processing").c_str());
        }
    }

    if(gcodeBuffer.isProcessed())
    {
        if(file.available())
        {
            const std::string line{ file.readStringUntil('\n').c_str() };
            currentLine++;
            if(line.starts_with(';') || line.length() == 0)
                ; // skip comments and empty lines
            else
                gcodeBuffer.setGcode(line);
            Serial.println(
            std::string(std::to_string(millis()) + " " + file.name() + ":" + std::to_string(currentLine) + ": '" + line + "'")
            .c_str());
        }
        else
        {
            Serial.println(std::string(std::to_string(millis()) + " file '" + file.name() + "' processed").c_str());
            operatingMode.switchState(OperatingState::State::FinishedFromFile);
            Serial.println(std::string(std::to_string(millis()) + " close file '" + file.name() + "'").c_str());
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
        Serial.println(std::string(std::to_string(millis()) + " failed to set new gcode file, try reset() first").c_str());
        return false;
    }

    filePath = path;
    return true;
}


const uint32_t &GcodeFileRunner::getCurrentLine() const { return currentLine; }

#endif
