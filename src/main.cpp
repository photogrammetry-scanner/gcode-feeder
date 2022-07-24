#if !defined(ENV_NATIVE)
#include "Resources.h"
#include <ArduinoOTA.h>
#include <Esp.h>
#include <elapsedMillis.h>

struct Firmware : public Resources
{
    void handleBufferedCncReception()
    {
        const uint8_t allowedSubsequentErrors{ 12 };

        if(operatingMode.isState(OperatingState::State::WaitFileCommandMotionFinished) ||
           operatingMode.isState(OperatingState::State::WaitHttpCommandMotionFinished))
            return;

        if(cncSerialBuffer.hasLine())
        {
            std::string line{ cncSerialBuffer.getLine() };
            if(!gcodeBuffer.isProcessed())
            {
                gcodeBuffer.setResponse(line);
            }
            else
                Serial.println(
                std::string(std::to_string(millis()) + " received unexpected cnc response '" + line + "'").c_str());

            if(operatingMode.isState(OperatingState::State::Idle) || operatingMode.isState(OperatingState::State::RunningFromFile))
            {
                display.screen.drawString(0, Display::L3, gcodeBuffer.getResponse().c_str());
                display.screen.display();
            }

            static uint8_t subsequentErrors;

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
                }
            }
            else // response OK
            {
                Serial.println(
                std::string(std::to_string(millis()) + " cnc response '" + gcodeBuffer.getResponse() + "'").c_str());
                subsequentErrors = 0;

                if(!gcodeBuffer.isMotionFinished() && operatingMode.isState(OperatingState::State::RunningFromFile))
                {
                    Serial.println(std::string(std::to_string(millis()) + " waiting for '" + gcodeBuffer.getGcode() + "' motion to finish")
                                   .c_str());
                    operatingMode.switchState(OperatingState::State::WaitFileCommandMotionFinished);
                }
                else if(!gcodeBuffer.isMotionFinished() && operatingMode.isState(OperatingState::State::Idle))
                {
                    Serial.println(std::string(std::to_string(millis()) + " waiting for '" + gcodeBuffer.getGcode() + "' motion to finish")
                                   .c_str());
                    operatingMode.switchState(OperatingState::State::WaitHttpCommandMotionFinished);
                }
                else if(operatingMode.isState(OperatingState::State::WaitingForCncControllerReady))
                {
                    operatingMode.switchState(OperatingState::State::Idle);
                }
            }

            if(operatingMode.isState(OperatingState::State::RunningFromFile))
            {
                display.screen.drawString(0, Display::L5,
                                          std::string("L " + std::to_string(gcodeFileRunner.getCurrentLine())).c_str());
                display.screen.display();
            }
        }
    }


    void handleBufferedGcodeTransmission()
    {
        if(!gcodeBuffer.isProcessed() && !gcodeBuffer.isTransmitted())
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

        static elapsedMillis elapsedTimeMs{ DELAY_MS_FOR_CNC_CONTROLLER_IS_READY_CHECK + 1 };
        if(elapsedTimeMs < DELAY_MS_FOR_CNC_CONTROLLER_IS_READY_CHECK)
            return;

        gcodeBuffer.setGcode("G91");
        handleBufferedGcodeTransmission();
        cncSerialBuffer.read();
        handleBufferedCncReception();

        if(gcodeBuffer.isResponseOk())
        {
            operatingMode.switchState(OperatingState::State::Idle);
            cncSerialBuffer.clear();
        }

        elapsedTimeMs = 0;
    }


    void handleStateWaitForMotionFinished()
    {
        static uint8_t pendingResponses{ 0 };

        if(!gcodeBuffer.isProcessed())
            return;

        if(!operatingMode.isState(OperatingState::State::WaitFileCommandMotionFinished) &&
           !operatingMode.isState(OperatingState::State::WaitHttpCommandMotionFinished))
            return;

        static elapsedMillis elapsedTimeMs;
        if(elapsedTimeMs <= DELAY_MS_FOR_MOTION_FINISHED_CHECK)
            return;
        elapsedTimeMs = 0;

        if(pendingResponses == 0)
        {
            Serial.println(std::string(std::to_string(millis()) + " send: '?'").c_str());
            cncSerial.write("?");
            pendingResponses++;
        }

        while(cncSerialBuffer.read()) {}

        if(cncSerialBuffer.hasLine())
        {
            if(pendingResponses > 0)
                pendingResponses--;

            const std::string line{ cncSerialBuffer.getLine() };
            Serial.println(std::string(std::to_string(millis()) + " cnc status: '" + line + "'").c_str());
            if(line.rfind("<Idle|", 0) != 0)
                return;
            else
            {
                Serial.println(std::string(std::to_string(millis()) + " motion finished").c_str());
                pendingResponses = 0;
                // cncSerialBuffer.clear();
            }
        }
        else
        {
            return;
        }

        gcodeBuffer.setMotionFinished();

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


    void fetchPendingBytesFromCncSerial() { cncSerialBuffer.read(); }


    void handleHaltStates()
    {
        if(operatingMode.isState(OperatingState::State::HaltOnError))
        {
            cncSerial.write("Q");
            while(true)
            {
                Serial.println(std::string(std::to_string(millis()) + " firmware halted").c_str());
                ESP.deepSleep(ESP.deepSleepMax());
            }
        }
    }

    void process()
    {
        handleHaltStates();
        fetchPendingBytesFromCncSerial();
        handleStateWaitForCncControllerReady();

        handleBufferedCncReception();
        handleStateRunningFromFile();
        handleBufferedGcodeTransmission();
        handleStateWaitForMotionFinished();

        handleStateDoResetWifi();
        handleStateDoReboot();
        ArduinoOTA.handle();
    }
} f;


void setup() { f.setup(); }

void loop() { f.process(); }

#endif
