#pragma once
#if !defined(ENV_NATIVE)
#include <FS.h>


class GcodeBuffer;
class OperatingState;


struct GcodeFileRunner
{
    GcodeFileRunner(GcodeBuffer &buffer, OperatingState &operatingMode);
    bool setFilepath(const std::string &filePath);
    void tryBufferNextLine();
    void reset();
    const uint32_t &getCurrentLine() const;

protected:
    std::string filePath;
    File file;
    uint32_t currentLine;
    GcodeBuffer &gcodeBuffer;
    OperatingState &operatingMode;
};

#endif
