#pragma once
#if !defined(ENV_NATIVE)
#include <string>

struct OperatingState
{
    enum struct State : uint8_t
    {
        Uninitialized,                 // default value after reset/reboot
        Setup,                         // initialization phase
        WaitingForCncControllerReady,  // sending G91 until 'ok' is received
        Idle,                          // accept gcode via http
        WaitHttpCommandMotionFinished, // wait until motion is finished (while processing in idle)
        WaitFileCommandMotionFinished, // wait until motion is finished (while processing file)
        RunningFromFile,               // accept gcode only from file
        PausedFromFile,                // on pause request while processing file
        FinishedFromFile,              // intermediate state before returning to idle
        DoResetWifi,                   // on reset wifi request
        DoReboot,                      // on reboot request
        Invalid,                       // no state, must not occur
    };

    bool switchState(State newState);
    static std::string toString(State state);
    std::string toString();
    bool isState(State state) const;
    State state() const;


protected:
    State currentState{ State::Invalid };
};

#endif
