#pragma once

#if defined(ENV_NATIVE)

int main(int argc, char **argv) { return tests(); }

#else

#include "Arduino.h"

void setup() { delay(1000); }

void loop()
{
    tests();
    delay(500);
}

#endif
