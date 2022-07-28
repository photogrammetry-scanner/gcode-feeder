#pragma once
#if !defined(ENV_NATIVE)
#include <string>


typedef struct GcodeBuffer
{
    [[nodiscard]] const std::string &getGcode() const;
    [[nodiscard]] const std::string &getResponse() const;
    void setGcode(const std::string &gcode);
    void setGcode(char gcode);
    void setResponse(const std::string &response);
    void setTransmitted(bool isTransmitted = true);
    // void setResponseReceived(bool isReceived = true);
    void setMotionFinished(bool isFinished = true);
    void reset();
    [[nodiscard]] bool isNone() const;
    [[nodiscard]] bool isProcessed() const;
    [[nodiscard]] bool isTransmitted() const;
    [[nodiscard]] bool isResponseOk() const;
    [[nodiscard]] bool isMotionFinished() const;
    [[nodiscard]] int16_t getErrorCode() const;
    [[nodiscard]] std::string toString() const;

protected:
    std::string gcode;
    std::string response;
    bool gcodeTransmitted{ false };
    bool responseReceived{ false };
    bool motionFinished{ true };
    bool hasGcodeSet{ false };

} GcodeBuffer;

#endif
