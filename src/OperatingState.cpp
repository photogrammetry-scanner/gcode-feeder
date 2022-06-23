#include "OperatingState.h"
#include <HardwareSerial.h>

bool OperatingState::switchState(OperatingState::State newState)
{
    bool hasSwitched{ currentState != newState };
    Serial.println("switch operating state: " + toString(currentState) + " -> " + toString(newState));
    currentState = newState;
    return hasSwitched;
}


String OperatingState::toString() { return toString(currentState); }


String OperatingState::toString(OperatingState::State s)
{
    switch(s)
    {
    case State::Undefined:
        return "Undefined";
    case State::Setup:
        return "Setup";
    case State::WaitingForCncController:
        return "WaitingForCncController";
    case State::Idle:
        return "Idle";
    case State::WaitCommandMotion:
        return "WaitCommandMotion";
    case State::RunningFromFile:
        return "RunningFromFile";
    case State::WaitCommandFromFileMotion:
        return "WaitCommandFromFileMotion";
    case State::PausedFromFile:
        return "PausedFromFile";
    case State::FinishedFromFile:
        return "FinishedFromFile";
    case State::Invalid:
        return "Invalid";
    case State::DoResetWifi:
        return "DoResetWifi";
    case State::DoReboot:
        return "DoReboot";
    default:
        return "UNKNOWN";
    }
}


OperatingState::State OperatingState::state() const { return currentState; }


bool OperatingState::isState(OperatingState::State s) const { return s == currentState; }
