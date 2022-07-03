#include "impl.h"


std::string getAndRemoveLine(std::string &buffer, bool inclusiveNewlineCharacter)
{
    auto nextNewlineIdx{ buffer.find('\n', 0) };

    if(nextNewlineIdx == std::string::npos)
    {
        return {};
    }
    else if(nextNewlineIdx == 0)
    {
        buffer.erase(0, 1);
        if(inclusiveNewlineCharacter)
            return std::string("\n");
        else
            return {};
    }
    else
    {
        std::string nextLine{ buffer.substr(0, nextNewlineIdx + 1) };
        buffer.erase(0, nextNewlineIdx + 1);

        if(!inclusiveNewlineCharacter)
            nextLine.erase(nextLine.length() - 1, 1);

        return nextLine;
    }

    return {};
}


bool bufferChar(std::string &buffer, char c)
{
    if(c == '\r')
        return false;

    buffer += c;

    if(c == '\n')
        return true;

    return false;
}
