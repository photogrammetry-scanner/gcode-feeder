#pragma once

#include "SSD1306Wire.h"
#include <stdint.h>
// #include <Wire.h>

typedef struct Display
{
    static const int16_t L1;
    static const int16_t L2;
    static const int16_t L3;
    static const int16_t L4;
    static const int16_t L5;
    SSD1306Wire screen = { 0x3C, SDA, SCL, GEOMETRY_64_48 };
} Display;
