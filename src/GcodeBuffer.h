#pragma once
#if !defined(ENV_NATIVE)
#include <string>


typedef struct GcodeBuffer
{
    const std::string &getGcode() const;
    const std::string &getResponse() const;
    void setGcode(const std::string &gcode);
    void setResponse(const std::string &response);
    void setTransmitted(bool isTransmitted = true);
    // void setResponseReceived(bool isReceived = true);
    void setMotionFinished(bool isFinished = true);
    void reset();
    bool isNone() const;
    bool isProcessed() const;
    bool isTransmitted() const;
    bool isResponseOk() const;
    bool isMotionFinished() const;
    int16_t getErrorCode() const;
    std::string toString() const;

protected:
    std::string gcode;
    std::string response;
    bool gcodeTransmitted{ false };
    bool responseReceived{ false };
    bool motionFinished{ true };
    bool hasGcodeSet{ false };

} GcodeBuffer;

#endif
