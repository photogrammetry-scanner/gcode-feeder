#pragma once
#if !defined(ENV_NATIVE)
#include <string>

class Stream;


typedef struct LineBufferedStream
{
    explicit LineBufferedStream(Stream &stream);
    bool read();
    void clear();
    bool hasLine() const;
    std::string getLine(bool inclusiveNewlineCharacter = false);

protected:
    Stream &stream;
    std::string buffer;

} LineBufferedSerial;
#endif
