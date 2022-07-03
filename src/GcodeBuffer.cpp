#if !defined(ENV_NATIVE)
#include "GcodeBuffer.h"


void GcodeBuffer::reset()
{
    gcode.clear();
    response.clear();
    gcodeTransmitted = false;
    responseReceived = false;
    motionFinished = true;
}


bool GcodeBuffer::isProcessed() const { return gcodeTransmitted && responseReceived; }


bool GcodeBuffer::isTransmitted() const { return gcodeTransmitted; }


void GcodeBuffer::setTransmitted(bool isTransmitted)
{
    gcodeTransmitted = isTransmitted;
    motionFinished = false;
}


void GcodeBuffer::setGcode(const std::string &g)
{
    reset();
    gcode = g;
}


void GcodeBuffer::setResponse(const std::string &r)
{
    response = r;
    responseReceived = true;
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


void GcodeBuffer::setResponseReceived(bool isReceived) { responseReceived = isReceived; }


bool GcodeBuffer::isMotionFinished() const { return motionFinished; }


void GcodeBuffer::setMotionFinished(bool isFinished) { motionFinished = isFinished; }

#endif
