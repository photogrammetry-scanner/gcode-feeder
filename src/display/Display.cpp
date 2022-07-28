#if !defined(ENV_NATIVE)
#include "Display.h"

void Display::setup()
{
    screen.init();
    screen.setColor(OLEDDISPLAY_COLOR::WHITE);
    screen.setFont(ArialMT_Plain_10);
    screen.flipScreenVertically();
    screen.setLogBuffer(6, 14);
    screen.clear();
    screen.display();
}

void Display::clear()
{
    screen.clear();
    for(uint16_t l{ L1 }; l < LINES_COUNT; l++)
        memset(buffer[l], ' ', LINE_CHARS - 1);
}

void Display::displayIfTouched()
{
    if(!isTouched)
        return;

    screen.clear();
    for(uint16_t l{ L1 }; l < LINES_COUNT; l++)
        screen.drawString(0, l * LINE_HEIGHT, buffer[l]);
    screen.display();

    isTouched = false;
}


void Display::printLine(uint16_t lineNr, const std::string &line)
{
    if(LINES_COUNT - 1 < lineNr)
        return;

    memset(buffer[lineNr], ' ', LINE_CHARS - 1);
    size_t numChars{ (line.length() >= LINE_CHARS) ? LINE_CHARS - 1 : line.length() };
    memcpy(buffer[lineNr], line.c_str(), numChars);

    isTouched = true;
};

#endif
