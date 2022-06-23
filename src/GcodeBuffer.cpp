#include "GcodeBuffer.h"
#include <stdio.h>


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


void GcodeBuffer::setGcode(const String &g)
{
    reset();
    gcode = g;
}


void GcodeBuffer::setResponse(const String &r)
{
    response = r;
    responseReceived = true;
}


const String &GcodeBuffer::getGcode() const { return gcode; }


const String &GcodeBuffer::getResponse() const { return response; }

#include <HardwareSerial.h>

bool GcodeBuffer::isResponseOk() const
{
    String r{ response };

    auto index = r.indexOf('\r');
    if(index >= 0)
        r.remove(index);

    index = r.indexOf('\n');
    if(index >= 0)
        r.remove(index);

    return 0 == r.compareTo("ok");
}


int16_t GcodeBuffer::getErrorCode() const
{
    String r{ response };

    auto index = r.indexOf('\r');
    if(index >= 0)
        r.remove(index);

    index = r.indexOf('\n');
    if(index >= 0)
        r.remove(index);

    int16_t code{ 0 };

    if(1 == sscanf(r.c_str(), "error:%hd", &code))
        return code;

    return -1;
}


void GcodeBuffer::setResponseReceived(bool isReceived) { responseReceived = isReceived; }


bool GcodeBuffer::isMotionFinished() const { return motionFinished; }


void GcodeBuffer::setMotionFinished(bool isFinished) { motionFinished = isFinished; }
