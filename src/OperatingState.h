#pragma once
#if !defined(ENV_NATIVE)
#include <WString.h>

struct OperatingState
{
    enum struct State : uint8_t
    {
        Undefined,                 // default
        Setup,                     // initialization phase
        WaitingForCncController,   // sending G91 until 'ok' is received
        Idle,                      // accept gcode via http
        WaitCommandMotion,         // wait until motion is finished (while processing in idle)
        RunningFromFile,           // accept gcode only from file
        WaitCommandFromFileMotion, // wait until motion is finished (while processing file)
        PausedFromFile,            // on pause request while processing file
        FinishedFromFile,          // intermediate state before returning to idle
        DoResetWifi,               // on reset wifi request
        DoReboot,                  // on reboot request
        Invalid,
    };

    bool switchState(State newState);
    static String toString(State state);
    String toString();
    bool isState(State state) const;
    State state() const;


protected:
    State currentState{ State::Invalid };
};

#endif
