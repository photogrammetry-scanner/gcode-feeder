#pragma once
#if !defined(ENV_NATIVE)
#include "SSD1306Wire.h"
#include <stdint.h>

// hardcoded config
#include "configuration.h"

typedef struct Display
{
    static const int16_t L1;
    static const int16_t L2;
    static const int16_t L3;
    static const int16_t L4;
    static const int16_t L5;
    SSD1306Wire screen = { DISPLAY_I2C_ADDRESS, DISPLAY_I2C_SDA_PIN, DISPLAY_I2C_SCL_PIN, DISPLAY_GEOMETRY };
} Display;

#endif
