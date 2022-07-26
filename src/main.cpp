#if !defined(ENV_NATIVE)
#include "Resources.h"
#include <ArduinoOTA.h>
#include <Esp.h>
#include <elapsedMillis.h>

struct Firmware : public Resources
{
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
        if(!gcodeBuffer.isProcessed())
        {
            gcodeBuffer.setResponse(line);
        }
        else
        {
            Serial.println(std::string(std::to_string(millis()) + " received unexpected cnc response '" + line + "'").c_str());
            return;
        }


        const uint8_t allowedSubsequentErrors{ 100 };
        static uint8_t subsequentErrors{ 0 };

        if(!gcodeBuffer.isResponseOk())
        {
            Serial.println(std::string(std::to_string(millis()) + " cnc response '" + gcodeBuffer.getResponse() +
                                       "' is erroneous (code=" + std::to_string(gcodeBuffer.getErrorCode()) + "), retransmit line")
                           .c_str());
            if(subsequentErrors >= allowedSubsequentErrors)
            {
                Serial.println(std::string(std::to_string(millis()) + " error: too many erroneous responses (" +
                                           std::to_string(subsequentErrors) + "), aborting")
                               .c_str());
                subsequentErrors = 0;
                operatingMode.switchState(OperatingState::State::HaltOnError);
            }
            else
            {
                subsequentErrors++;
                const std::string gcode{ gcodeBuffer.getGcode() };
                gcodeBuffer.setGcode(gcode);
                cncSerialBuffer.clear();
                delay(100);
            }
            return;
        }

        Serial.println(std::string(std::to_string(millis()) + " cnc response '" + gcodeBuffer.getResponse() + "'").c_str());
        subsequentErrors = 0;


        if(!gcodeBuffer.isMotionFinished())
        {
            if(operatingMode.isState(OperatingState::State::RunningFromFile))
                operatingMode.switchState(OperatingState::State::WaitFileCommandMotionFinished);
            if(operatingMode.isState(OperatingState::State::Idle))
                operatingMode.switchState(OperatingState::State::WaitHttpCommandMotionFinished);
        }
    }


    void handleBufferedGcodeTransmission()
    {
        if(!gcodeBuffer.isNone() && !gcodeBuffer.isProcessed() && !gcodeBuffer.isTransmitted())
        {
            Serial.println(std::string(std::to_string(millis()) + " send gcode='" + gcodeBuffer.getGcode() + "'").c_str());
            if(operatingMode.isState(OperatingState::State::Idle) || operatingMode.isState(OperatingState::State::RunningFromFile))
            {
                display.screen.clear();
                display.screen.drawString(0, Display::L1, "gcode->cnc");
                display.screen.drawString(0, Display::L2, gcodeBuffer.getGcode().c_str());
                display.screen.display();
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
            gcodeBuffer.setGcode("G91");
            return;
        }

        if(!gcodeBuffer.isProcessed() || !gcodeBuffer.isResponseOk())
        {
            gcodeBuffer.setGcode("G91");
            return;
        }

        if(gcodeBuffer.isProcessed() && gcodeBuffer.isResponseOk())
        {
            cncSerialBuffer.read();
            cncSerialBuffer.clear();
            operatingMode.switchState(OperatingState::State::Idle);
        }
    }


    /**
     * Sends "?" to request status and watches for "<Idle.*".
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
            cncSerial.println("?"); // triggers two responses: "ok" + "<Run..." or "<Idle..."
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
        cncSerialBuffer.read();
        cncSerialBuffer.clear();

        if(operatingMode.isState(OperatingState::State::WaitFileCommandMotionFinished))
            operatingMode.switchState(OperatingState::State::RunningFromFile);

        if(operatingMode.isState(OperatingState::State::WaitHttpCommandMotionFinished))
            operatingMode.switchState(OperatingState::State::Idle);
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
        if(operatingMode.isState(OperatingState::State::HaltOnSetupFailed) || operatingMode.isState(OperatingState::State::HaltOnError))
        {
            cncSerial.println("Q");
            while(true)
            {
                Serial.println(std::string(std::to_string(millis()) + " firmware halted").c_str());
                EspClass::deepSleep(EspClass::deepSleepMax());
            }
        }
    }


    void handleStateDoResetCncController()
    {
        if(!operatingMode.isState(OperatingState::State::DoResetCncController))
            return;

        static bool preResetCondition{ false };
        waitUntilControllerResponsive(preResetCondition, OperatingState::State::AnyState, OperatingState::State::DoResetCncController);
        if(!preResetCondition)
            return;

        static bool isFirst{ true };
        if(isFirst)
        {
            isFirst = false;
            Serial.println(std::string(std::to_string(millis()) + " resetting controller ...").c_str());
            Serial.println(std::string(std::to_string(millis()) + " send '$RST=*'").c_str());
            cncSerialBuffer.read();
            cncSerialBuffer.clear();
            cncSerial.println("$RST=*");
            return;
        }

        static bool postResetCondition{ false };
        waitUntilControllerResponsive(postResetCondition, OperatingState::State::AnyState,
                                      OperatingState::State::WaitingForCncControllerReady);

        if(!postResetCondition)
            return;

        cncSerialBuffer.read();
        cncSerialBuffer.clear();
    }


    /**
     * Waits until the controller is responsive.
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
        cncSerialBuffer.read();
        if(!cncSerialBuffer.hasLine())
        {
            Serial.println(std::string(std::to_string(millis()) + " send 'G91'").c_str());
            cncSerial.println("G91");
            return;
        }

        const std::string line{ cncSerialBuffer.getLine() };
        if(line.starts_with("ok"))
        {
            Serial.println(std::string(std::to_string(millis()) + " received 'ok'").c_str());
            condition_out = true;
            operatingMode.switchState(nextState);
            return;
        }
    };


    void process()
    {
        cncSerialBuffer.read();

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
    f.operatingMode.switchState(OperatingState::State::DoResetCncController);
}

void loop() { f.process(); }

#endif
