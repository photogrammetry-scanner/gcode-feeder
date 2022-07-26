#if !defined(ENV_NATIVE)
#include "GcodeBuffer.h"
#include <HardwareSerial.h>

void GcodeBuffer::reset()
{
    gcode.clear();
    response.clear();
    gcodeTransmitted = false;
    responseReceived = false;
    motionFinished = true;
    hasGcodeSet = false;
}


bool GcodeBuffer::isNone() const { return !hasGcodeSet; }


bool GcodeBuffer::isProcessed() const { return gcodeTransmitted && responseReceived; }


bool GcodeBuffer::isTransmitted() const { return gcodeTransmitted; }


void GcodeBuffer::setTransmitted(bool isTransmitted)
{
    gcodeTransmitted = isTransmitted;
    motionFinished = false;
    Serial.println(std::string(std::to_string(millis()) + " GcodeBuffer::setTransmitted:   " + toString() + "").c_str());
}


void GcodeBuffer::setGcode(const std::string &g)
{
    reset();
    gcode = g;
    hasGcodeSet = true;
    Serial.println(std::string(std::to_string(millis()) + " GcodeBuffer::setGcode:         " + toString() + "").c_str());
}


void GcodeBuffer::setResponse(const std::string &r)
{
    response = r;
    responseReceived = true;
    Serial.println(std::string(std::to_string(millis()) + " GcodeBuffer::setResponse:      " + toString() + "").c_str());
}


const std::string &GcodeBuffer::getGcode() const { return gcode; }


const std::string &GcodeBuffer::getResponse() const { return response; }


bool GcodeBuffer::isResponseOk() const
{
    std::string r{ response };

    auto index = r.find('\r');
    if((index != std::string::npos) && (index > 0))
        r.erase(index);

    index = r.find('\n');
    if((index != std::string::npos) && (index > 0))
        r.erase(index);

    return "ok" == r;
}


int16_t GcodeBuffer::getErrorCode() const
{
    std::string r{ response };

    auto index = r.find('\r');
    if((index != std::string::npos) && (index > 0))
        r.erase(index);

    index = r.find('\n');
    if((index != std::string::npos) && (index > 0))
        r.erase(index);

    int16_t code{ 0 };

    if(1 == sscanf(r.c_str(), "error:%hd", &code))
        return code;

    return -1;
}


/*
void GcodeBuffer::setResponseReceived(bool isReceived)
{
    responseReceived = isReceived;
    if(!responseReceived)
        response = "";
    Serial.println(std::string(std::to_string(millis()) + " GcodeBuffer::setResponseReceived: " + toString() + "").c_str());
}
*/

bool GcodeBuffer::isMotionFinished() const { return motionFinished; }


void GcodeBuffer::setMotionFinished(bool isFinished)
{
    motionFinished = isFinished;
    Serial.println(std::string(std::to_string(millis()) + " GcodeBuffer::isMotionFinished: " + toString() + "").c_str());
}


std::string GcodeBuffer::toString() const
{
    // clang-format off
    return std::string
    {
        "TX="  + std::string(gcodeTransmitted ? "yes" : "no ") + " " +
        "RX="  + std::string(responseReceived ? "yes" : "no ") + " " +
        "MvFin=" + std::string(motionFinished ? "yes" : "no ") + " " +
        "G='"   + gcode    + "' " +
        "Rsp='" + response + "'"
    };
    // clang-format on
}

#endif
