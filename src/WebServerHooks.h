#pragma once
#if !defined(ENV_NATIVE)
#include "Resources.h"

struct WebServerHooks
{
    static void setup(Resources &r);
};

#endif
