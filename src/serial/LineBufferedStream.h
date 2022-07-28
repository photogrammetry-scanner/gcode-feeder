#pragma once
#if !defined(ENV_NATIVE)
#include <string>

class Stream;


typedef struct LineBufferedStream
{
    explicit LineBufferedStream(Stream &stream);
    bool read();
    void clear();
    void flush();
    [[nodiscard]] bool hasLine() const;
    std::string getLine(bool inclusiveNewlineCharacter = false);

protected:
    Stream &stream;
    std::string buffer;

} LineBufferedStream;
#endif
