#pragma once
#if !defined(ENV_NATIVE)
#include "SSD1306Wire.h"
#include <stdint.h>

// hardcoded config
#include "configuration.h"

typedef struct Display
{
    static constexpr uint16_t LINE_HEIGHT{ 9 };
    static constexpr uint16_t LINES_COUNT{ 5 };
    static constexpr uint16_t LINE_CHARS{ 30 };
    static constexpr uint16_t L1{ 0 };
    static constexpr uint16_t L2{ 1 };
    static constexpr uint16_t L3{ 2 };
    static constexpr uint16_t L4{ 3 };
    static constexpr uint16_t L5{ 4 };

public:
    SSD1306Wire screen = { DISPLAY_I2C_ADDRESS, DISPLAY_I2C_SDA_PIN, DISPLAY_I2C_SCL_PIN, DISPLAY_GEOMETRY };

    void setup();
    void printLine(uint16_t lineNr, const std::string &line);
    void clear();
    void displayIfTouched();

private:
    char buffer[LINES_COUNT][LINE_CHARS]{ { 0 } };
    bool isTouched{ false };
} Display;

#endif
