#if !defined(ENV_NATIVE)
#include "LineBufferedStream.h"
#include "impl.h"
#include <Stream.h>

LineBufferedStream::LineBufferedStream(Stream &s) : stream(s) {}


bool LineBufferedStream::read()
{
    bool hasRead{ false };
    while(stream.available())
    {
        int i = stream.read();
        if(i < 0)
            return false;

        char c = static_cast<char>(i);
        if(bufferChar(buffer, c))
            hasRead = true;
    }
    return hasRead;
}


void LineBufferedStream::clear() { buffer.clear(); }


void LineBufferedStream::flush()
{
    read();
    clear();
}

bool LineBufferedStream::hasLine() const { return buffer.find('\n', 0) != std::string::npos; }


std::string LineBufferedStream::getLine(bool inclusiveNewlineCharacter)
{
    return getAndRemoveLine(buffer, inclusiveNewlineCharacter);
}

#endif
