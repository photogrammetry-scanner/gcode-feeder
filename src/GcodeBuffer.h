#pragma once
#if !defined(ENV_NATIVE)
#include <WString.h>


typedef struct GcodeBuffer
{
    const String &getGcode() const;
    const String &getResponse() const;
    void setGcode(const String &gcode);
    void setResponse(const String &response);
    void setTransmitted(bool isTransmitted = true);
    void setResponseReceived(bool isReceived = true);
    void setMotionFinished(bool isFinished = true);
    void reset();
    bool isProcessed() const;
    bool isTransmitted() const;
    bool isResponseOk() const;
    bool isMotionFinished() const;
    int16_t getErrorCode() const;

protected:
    String gcode;
    String response;
    bool gcodeTransmitted{ false };
    bool responseReceived{ false };
    bool motionFinished{ true };

} GcodeBuffer;

#endif
