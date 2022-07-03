#if !defined(ENV_NATIVE)
#include "GcodeFileRunner.h"
#include "GcodeBuffer.h"
#include "OperatingState.h"
#include <LittleFS.h>
#include <WString.h>


GcodeFileRunner::GcodeFileRunner(GcodeBuffer &buffer, OperatingState &opMode)
: filePath("test.g"), currentLine(0), gcodeBuffer(buffer), operatingMode(opMode)
{
}


void GcodeFileRunner::process()
{
    static String line;

    if(file && !operatingMode.isState(OperatingState::State::RunningFromFile))
    {
        Serial.println(String() + "close file '" + file.name() + "'");
        currentLine = 0;
        file.close();
    }

    if(!file)
    {
        Serial.println("gcode file '" + filePath + "' not opened");
        file = LittleFS.open(filePath, "r");
        if(file.isDirectory())
        {
            Serial.println(String() + "file '" + file.name() + "' is a directory, closing file");
            file.close();
        }
        else
        {
            Serial.println(String() + "gcode file '" + file.name() + "' opened for processing");
        }
    }

    if(gcodeBuffer.isProcessed())
    {
        if(file.available())
        {
            gcodeBuffer.setGcode(file.readStringUntil('\n'));
            currentLine++;
            Serial.println(String() + "processing line " + currentLine + ": '" + gcodeBuffer.getGcode() + "'");
        }
        else
        {
            Serial.println(String() + "file '" + file.name() + "' processed");
            operatingMode.switchState(OperatingState::State::FinishedFromFile);
            Serial.println(String() + "close file '" + file.name() + "'");
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


bool GcodeFileRunner::setFilepath(const String &path)
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
