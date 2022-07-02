#include "LineBufferedStream.h"
#include <Stream.h>


LineBufferedStream::LineBufferedStream(Stream &serial) : stream(serial) {}


bool LineBufferedStream::read()
{
    while(stream.available())
    {
        int i = stream.read();
        if(i < 0)
            return false;

        char c = static_cast<char>(i);
        if(c == '\r')
            continue;

        buffer += c;

        if(c == '\n')
        {
            return true;
        }
    }
    return false;
}


void LineBufferedStream::clear() { buffer.clear(); }


bool LineBufferedStream::hasLine() const { return buffer.indexOf('\n', 0) >= 0; }


String LineBufferedStream::getLine(bool inclusiveNewlineCharacter)
{
    int nextNewlineIdx{ buffer.indexOf('\n') };

    if(nextNewlineIdx < 0)
        return {};

    if(nextNewlineIdx == 0)
    {
        buffer.remove(0, 1);
        if(inclusiveNewlineCharacter)
            return { "\n" };
        else
            return {};
    }

    if(nextNewlineIdx > 0)
    {
        String nextLine{ buffer.substring(0, nextNewlineIdx + 1) };
        buffer.remove(0, nextNewlineIdx + 1);

        if(!inclusiveNewlineCharacter)
            nextLine.remove(nextLine.length() - 1, 1);

        return nextLine;
    }

    return {};
}
