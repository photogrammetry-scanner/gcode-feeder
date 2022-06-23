#pragma once

#include <WString.h>

class Stream;

typedef struct LineBufferedStream
{
    LineBufferedStream(Stream &serial);
    bool read();
    void clear();
    bool hasLine() const;
    String getLine(bool inclusiveNewlineCharacter = false);

protected:
    Stream &stream;
    String buffer;

} LineBufferedSerial;
