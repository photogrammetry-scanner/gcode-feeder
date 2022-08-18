#if !defined(ENV_NATIVE)
#include "Resources.h"
#include <ArduinoOTA.h>
#include <Esp.h>
#include <elapsedMillis.h>

struct Firmware : public Resources
{
    void switchState(OperatingState::State nextState)
    {
        operatingMode.switchState(nextState);
        display.printLine(Display::L5, operatingMode.toString());
    }

    /**
     * Try to interpret received lines from CNC controller while in normal operating modes.
     * Received lines are responses to transmissions from g-code buffer (automatic status reports).
     */
    void handleBufferedCncReception()
    {
        if(!operatingMode.isState(OperatingState::State::WaitingForCncControllerReady) &&
           !operatingMode.isState(OperatingState::State::Idle) && !operatingMode.isState(OperatingState::State::RunningFromFile) &&
           !operatingMode.isState(OperatingState::State::PausedFromFile) &&
           !operatingMode.isState(OperatingState::State::FinishedFromFile))
            return;

        if(!cncSerialBuffer.hasLine())
            return;

        std::string line{ cncSerialBuffer.getLine() };
        auto printStatus = [&]()
        {
            display.printLine(Display::L3, "R " + line);
            const auto ec{ gcodeBuffer.getErrorCode() };
            display.printLine(Display::L4, "E " + ((ec < 0) ? "-" : std::to_string(ec)));
        };

        if(!gcodeBuffer.isProcessed())
        {
            gcodeBuffer.setResponse(line);
        }
        else
        {
            Serial.println(std::string(std::to_string(millis()) + " received unexpected cnc response '" + line + "'").c_str());
            printStatus();
            return;
        }

        const uint8_t allowedSubsequentErrors{ 100 };
        static uint8_t subsequentErrors{ 0 };

        if(!gcodeBuffer.isResponseOk())
        {
            Serial.println(std::string(std::to_string(millis()) + " cnc response '" + gcodeBuffer.getResponse() +
                                       "' is erroneous (code=" + std::to_string(gcodeBuffer.getErrorCode()) + "), retransmit line")
                           .c_str());
            printStatus();
            if(subsequentErrors >= allowedSubsequentErrors)
            {
                Serial.println(std::string(std::to_string(millis()) + " error: too many erroneous responses (" +
                                           std::to_string(subsequentErrors) + "), aborting")
                               .c_str());
                subsequentErrors = 0;
                switchState(OperatingState::State::DoHaltOnResponseError);
            }
            else
            {
                subsequentErrors++;
                const std::string gcode{ gcodeBuffer.getGcode() };
                gcodeBuffer.setGcode(gcode);
                // cncSerialBuffer.clear();
                delay(100);
            }
            return;
        }

        Serial.println(std::string(std::to_string(millis()) + " cnc response '" + gcodeBuffer.getResponse() + "'").c_str());
        subsequentErrors = 0;
        printStatus();

        if(gcodeBuffer.getGcode().starts_with('$')) // '$' commands trigger no motions
            gcodeBuffer.setMotionFinished(true);

        if(!gcodeBuffer.isMotionFinished())
        {
            if(operatingMode.isState(OperatingState::State::RunningFromFile))
                switchState(OperatingState::State::WaitFileCommandMotionFinished);
            if(operatingMode.isState(OperatingState::State::Idle))
                switchState(OperatingState::State::WaitHttpCommandMotionFinished);
        }
    }

    /**
     * Transmit buffered g-code.
     */
    void handleBufferedGcodeTransmission()
    {
        if(!gcodeBuffer.isNone() && !gcodeBuffer.isProcessed() && !gcodeBuffer.isTransmitted())
        {
            Serial.println(std::string(std::to_string(millis()) + " send gcode='" + gcodeBuffer.getGcode() + "'").c_str());
            if(operatingMode.isState(OperatingState::State::Idle) || operatingMode.isState(OperatingState::State::RunningFromFile))
            {
                display.printLine(Display::L1, (!gcodeBuffer.isProcessed()) ? gcodeBuffer.getGcode() : "");
                display.printLine(Display::L2, (operatingMode.isState(OperatingState::State::RunningFromFile)) ?
                                               std::to_string(gcodeFileRunner.getCurrentLine()) :
                                               "");
            }
            cncSerial.print(std::string(gcodeBuffer.getGcode() + '\n').c_str());
            gcodeBuffer.setTransmitted();
        }
    }


    void handleStateRunningFromFile()
    {
        if(!operatingMode.isState(OperatingState::State::RunningFromFile))
            return;

        static elapsedMillis elapsedTimeMs{ DELAY_MS_FOR_NEXT_LINE_FROM_FILE_CHECK + 1 };
        if(elapsedTimeMs >= DELAY_MS_FOR_NEXT_LINE_FROM_FILE_CHECK)
        {
            gcodeFileRunner.tryBufferNextLine();
            elapsedTimeMs = 0;
        }
    }


    void handleStateWaitForCncControllerReady()
    {
        if(!operatingMode.isState(OperatingState::State::WaitingForCncControllerReady))
            return;

        static elapsedMillis elapsedTimeMs{ DELAY_MS_FOR_CNC_CONTROLLER_IS_READY_CHECK };
        if(elapsedTimeMs < DELAY_MS_FOR_CNC_CONTROLLER_IS_READY_CHECK)
            return;
        elapsedTimeMs = 0;

        if(!gcodeBuffer.isTransmitted())
        {
            gcodeBuffer.setGcode(GRBL_CMD_STATUS_REPORT);
            return;
        }

        if(!gcodeBuffer.isProcessed())
        {
            return;
        }

        if(gcodeBuffer.isProcessed() && !gcodeBuffer.isResponseOk())
        {
            gcodeBuffer.setGcode(GRBL_CMD_STATUS_REPORT);
            cncSerialBuffer.flush();
            return;
        }

        if(gcodeBuffer.isProcessed() && gcodeBuffer.isResponseOk())
        {
            // cncSerialBuffer.flush();
            switchState(OperatingState::State::Idle);
        }
    }


    /**
     * Request status report and watches for "<Idle.*".
     * Additionally forces .read() from CNC serial to reduce latency.
     * Higher latency is will harm performance when processing from file.
     */
    void handleStateWaitForMotionFinished()
    {
        if(!operatingMode.isState(OperatingState::State::WaitFileCommandMotionFinished) &&
           !operatingMode.isState(OperatingState::State::WaitHttpCommandMotionFinished))
            return;

        if(!gcodeBuffer.isProcessed())
            return;

        static elapsedMillis elapsedTimeMs{ DELAY_MS_FOR_MOTION_FINISHED_CHECK };
        if(elapsedTimeMs < DELAY_MS_FOR_MOTION_FINISHED_CHECK)
            return;
        elapsedTimeMs = 0;

        static uint8_t pendingResponses{ 0 };
        if(pendingResponses == 0)
        {
            cncSerial.println(GRBL_CMD_STATUS_REPORT); // triggers two responses: "ok" + "<Run..." or "<Idle..."
            pendingResponses++;
        }

        cncSerialBuffer.read();
        if(!cncSerialBuffer.hasLine())
            return;

        bool motionFinished{ false };
        while(cncSerialBuffer.hasLine())
        {
            const std::string line{ cncSerialBuffer.getLine() };
            if(line.starts_with("ok")) {}
            else
            {
                if(pendingResponses > 0)
                    pendingResponses--;
                if(line.starts_with("<Idle|"))
                {
                    Serial.println(std::string(std::to_string(millis()) + " cnc status: '" + line + "'").c_str());
                    motionFinished = true;
                    break;
                }
            }
        }

        if(!motionFinished)
            return;

        pendingResponses = 0;
        gcodeBuffer.setMotionFinished();
        cncSerialBuffer.flush();

        if(operatingMode.isState(OperatingState::State::WaitFileCommandMotionFinished))
            switchState(OperatingState::State::RunningFromFile);

        if(operatingMode.isState(OperatingState::State::WaitHttpCommandMotionFinished))
            switchState(OperatingState::State::Idle);
    }


    void handleStateDoResetWifi()
    {
        if(!operatingMode.isState(OperatingState::State::DoResetWifi))
            return;
        AsyncWiFiManager wifiManager(&webServer, &dnsServer);
        wifiManager.setDebugOutput(false);
        wifiManager.resetSettings();
        EspClass::restart();
    }


    void handleStateDoReboot()
    {
        if(!operatingMode.isState(OperatingState::State::DoReboot))
            return;
        EspClass::restart();
    }


    void handleHaltStates()
    {
        if(operatingMode.isState(OperatingState::State::DoHaltOnSetupFailed) ||
           operatingMode.isState(OperatingState::State::DoHaltOnResponseError) ||
           operatingMode.isState(OperatingState::State::DoHaltOnError))
        {
            cncSerial.println(GRBL_CMD_RESET);
            while(true)
            {
                Serial.println(std::string(std::to_string(millis()) + " firmware halted").c_str());
                display.clear();
                display.printLine(Display::L1, "halted");
                display.printLine(Display::L5, operatingMode.toString());
                display.displayIfTouched();
                EspClass::deepSleep(EspClass::deepSleepMax());
            }
        }
    }


    /**
     * Reset the CNC controller:
     *  1. waits until controller is responsive
     *  2. sends '$RST=*\r\n', controller then resets settings and reboots
     *  3. waits until controller is responsive again
     */
    void handleStateDoResetCncController()
    {
        if(!operatingMode.isState(OperatingState::State::DoResetCncController))
            return;

        static bool isFirst2{ true };
        if(isFirst2)
        {
            isFirst2 = false;
            Serial.println(std::string(std::to_string(millis()) + " rebooting controller ...").c_str());
            Serial.println(std::string(std::to_string(millis()) + " send '" + GRBL_CMD_RESET + " \\r\\n'").c_str());
            cncSerial.printf("%c\r\n", GRBL_CMD_RESET);
            delay(200);
            cncSerialBuffer.flush();
            return;
        }

        static bool preResetCondition{ false };
        waitUntilControllerResponsive(preResetCondition, OperatingState::State::AnyState, OperatingState::State::DoResetCncController);
        if(!preResetCondition)
            return;

        static bool isFirst{ true };
        if(isFirst)
        {
            isFirst = false;
            Serial.println(std::string(std::to_string(millis()) + " resetting controller ...").c_str());
            Serial.println(std::string(std::to_string(millis()) + " send '" + GRBL_RESTORE_EEPROM_WIPE_ALL + "'").c_str());
            cncSerial.println(GRBL_RESTORE_EEPROM_WIPE_ALL);
            delay(200);
            cncSerialBuffer.flush();
            return;
        }

        static bool postResetCondition{ false };
        waitUntilControllerResponsive(postResetCondition, OperatingState::State::AnyState,
                                      OperatingState::State::WaitingForCncControllerReady);

        if(!postResetCondition)
            return;

        cncSerialBuffer.flush();
    }


    /**
     * Waits until the controller is responsive: sends \r\n until response 'OK' is received.
     * Requires `condition == false` and `operatingMode.state() == triggerState`.
     * @param condition shall be false at start; if controller is ready condition is set to true
     * @param triggerState required state or OperatingState::State::AnySate
     * @param nextState next state to jump to; can be same state again
     * @param separationMs time separation in between checks
     */
    void waitUntilControllerResponsive(bool &condition_out,
                                       OperatingState::State triggerState,
                                       OperatingState::State nextState,
                                       size_t separationMs = DELAY_MS_FOR_CNC_CONTROLLER_IS_READY_CHECK)
    {
        if(condition_out)
            return;

        if(operatingMode.state() != triggerState && triggerState != OperatingState::State::AnyState)
            return;

        static elapsedMillis elapsedTimeMs{ separationMs };
        if(separationMs > 0)
        {
            if(elapsedTimeMs < separationMs)
                return;
            elapsedTimeMs = 0;
        }

        Serial.println(std::string(std::to_string(millis()) + " waiting for 'ok' in state " + operatingMode.toString() +
                                   " (transition:" + OperatingState::toString(triggerState) + " -> " +
                                   OperatingState::toString(nextState) + ") ...")
                       .c_str());

        if(!cncSerialBuffer.hasLine())
        {
            Serial.println(std::string(std::to_string(millis()) + " send '\\r\\n'").c_str());
            cncSerial.print("\r\n");
            return;
        }

        const std::string line{ cncSerialBuffer.getLine() };
        if(line == "ok")
        {
            Serial.println(std::string(std::to_string(millis()) + " controller responded 'ok'").c_str());
            condition_out = true;
            switchState(nextState);
            cncSerialBuffer.flush();
            return;
        }
    };


    void process()
    {
        cncSerialBuffer.read();
        display.displayIfTouched();

        handleHaltStates();
        handleBufferedCncReception();

        handleStateDoResetCncController();
        handleStateWaitForCncControllerReady();

        handleStateRunningFromFile();
        handleBufferedGcodeTransmission();
        handleStateWaitForMotionFinished();

        handleStateDoResetWifi();
        handleStateDoReboot();

        ArduinoOTA.handle();
    }
} f;


void setup()
{
    f.setup();
    f.handleHaltStates();
    f.switchState(OperatingState::State::DoResetCncController);
}

void loop() { f.process(); }

#endif
