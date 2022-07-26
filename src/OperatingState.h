#pragma once
#if !defined(ENV_NATIVE)
#include <string>

struct OperatingState
{
    enum struct State : uint8_t
    {
        Uninitialized, // default value after reset/reboot

        // initialization states
        Setup,                        // initialization phase
        SetupFinished,                // initialization done
        DoResetCncController,         // resetting CNC controller to defaults
        WaitingForCncControllerReady, // sending G91 until 'ok' is received

        // operational states with g-code transfer
        Idle,                          // accept gcode via http
        WaitHttpCommandMotionFinished, // wait until motion is finished (while processing in idle)
        WaitFileCommandMotionFinished, // wait until motion is finished (while processing file)
        RunningFromFile,               // accept gcode only from file
        PausedFromFile,                // on pause request while processing file
        FinishedFromFile,              // intermediate state before returning to idle

        // post operational states followed by reboot
        DoResetWifi,       // on reset wifi request
        HaltOnSetupFailed, // halt system due initialization error
        HaltOnError,       // halt system due to error
        DoReboot,          // on reboot request

        // no real states
        AnyState, // dummy state to indicate some state
        Invalid,  // no state, must not occur
    };

    bool switchState(State newState);
    static std::string toString(State state);
    std::string toString();
    [[nodiscard]] bool isState(State state) const;
    [[nodiscard]] State state() const;


protected:
    State currentState{ State::Invalid };
};

#endif
