#if !defined(ENV_NATIVE)
#include "OperatingState.h"
#include <HardwareSerial.h>

bool OperatingState::switchState(OperatingState::State newState)
{
    bool hasSwitched{ currentState != newState };
    Serial.println(std::string(std::to_string(millis()) + " switch operating state: " + toString(currentState) + " -> " + toString(newState))
                   .c_str());
    currentState = newState;
    return hasSwitched;
}


std::string OperatingState::toString() { return toString(currentState); }


std::string OperatingState::toString(OperatingState::State s)
{
    switch(s)
    {
    case State::Uninitialized:
        return "Uninitialized";
    case State::Setup:
        return "Setup";
    case State::WaitingForCncControllerReady:
        return "WaitingForCncControllerReady";
    case State::Idle:
        return "Idle";
    case State::WaitHttpCommandMotionFinished:
        return "WaitHttpCommandMotionFinished";
    case State::RunningFromFile:
        return "RunningFromFile";
    case State::WaitFileCommandMotionFinished:
        return "WaitFileCommandMotionFinished";
    case State::PausedFromFile:
        return "PausedFromFile";
    case State::FinishedFromFile:
        return "FinishedFromFile";
    case State::Invalid:
        return "Invalid";
    case State::DoResetWifi:
        return "DoResetWifi";
    case State::HaltOnError:
        return "HaltOnError";
    case State::DoReboot:
        return "DoReboot";
    default:
        return "UNKNOWN";
    }
}


OperatingState::State OperatingState::state() const { return currentState; }


bool OperatingState::isState(OperatingState::State s) const { return s == currentState; }

#endif
