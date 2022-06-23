#pragma once

#include <FS.h>
#include <WString.h>


class GcodeBuffer;
class OperatingState;


struct GcodeFileRunner
{
    GcodeFileRunner(GcodeBuffer &buffer, OperatingState &operatingMode);
    bool setFilepath(const String &filePath);
    void process();
    void reset();
    const uint32_t &getCurrentLine() const;

protected:
    String filePath;
    File file;
    uint32_t currentLine;
    GcodeBuffer &gcodeBuffer;
    OperatingState &operatingMode;
};
