#if !defined(ENV_NATIVE)
#include "Resources.h"
#include <Esp.h>
#include <elapsedMillis.h>


struct Firmware : public Resources
{
    void handleCncResponse()
    {
        const uint8_t allowedSubsequentErrors{ 4 };

        if(operatingMode.isState(OperatingState::State::WaitCommandFromFileMotion) ||
           operatingMode.isState(OperatingState::State::WaitCommandMotion))
            return;

        if(cncSerialBuffer.hasLine())
        {
            std::string line{ cncSerialBuffer.getLine() };
            if(!gcodeBuffer.isProcessed())
            {
                gcodeBuffer.setResponse(line);
            }
            else
                Serial.println(std::string("received unexpected cnc response '" + line + "'").c_str());

            if(operatingMode.isState(OperatingState::State::Idle) || operatingMode.isState(OperatingState::State::RunningFromFile))
            {
                display.screen.drawString(0, Display::L3, gcodeBuffer.getResponse().c_str());
                display.screen.display();
            }

            static uint8_t subsequentErrors;

            if(!gcodeBuffer.isResponseOk())
            {
                Serial.println(std::string("cnc response '" + gcodeBuffer.getResponse() +
                                           "' is erroneous (code=" + std::to_string(gcodeBuffer.getErrorCode()) + "), retransmit line")
                               .c_str());
                if(subsequentErrors >= allowedSubsequentErrors)
                {
                    Serial.println(std::string("error: too many erroneous responses (" + std::to_string(subsequentErrors) + "), aborting")
                                   .c_str());
                    subsequentErrors = 0;
                    operatingMode.switchState(OperatingState::State::WaitingForCncController);
                }
                subsequentErrors++;
                gcodeBuffer.setTransmitted(false);
                gcodeBuffer.setResponseReceived(false);
            }
            else // response OK
            {
                Serial.println(std::string("cnc response '" + gcodeBuffer.getResponse() + "'").c_str());
                subsequentErrors = 0;

                if(!gcodeBuffer.isMotionFinished() && operatingMode.isState(OperatingState::State::RunningFromFile))
                {
                    Serial.println(std::string("waiting for '" + gcodeBuffer.getGcode() + "' motion to finish").c_str());
                    operatingMode.switchState(OperatingState::State::WaitCommandFromFileMotion);
                }
                else if(!gcodeBuffer.isMotionFinished() && operatingMode.isState(OperatingState::State::Idle))
                {
                    Serial.println(std::string("waiting for '" + gcodeBuffer.getGcode() + "' motion to finish").c_str());
                    operatingMode.switchState(OperatingState::State::WaitCommandMotion);
                }
                else if(operatingMode.isState(OperatingState::State::WaitingForCncController))
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


    void sendBufferedGcode()
    {
        if(!gcodeBuffer.isProcessed() && !gcodeBuffer.isTransmitted())
        {
            Serial.println(std::string("send gcode='" + gcodeBuffer.getGcode() + "'").c_str());
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


    void feedGcodeBufferFromFile()
    {
        static elapsedMillis elapsedTimeMs{ DELAY_MS_FOR_NEXT_LINE_FROM_FILE_CHECK + 1 };

        if(!operatingMode.isState(OperatingState::State::RunningFromFile))
            return;

        if(elapsedTimeMs >= DELAY_MS_FOR_NEXT_LINE_FROM_FILE_CHECK)
        {
            gcodeFileRunner.process();
            elapsedTimeMs = 0;
        }
    }

    void waitForCncControllerReady()
    {
        static elapsedMillis elapsedTimeMs{ DELAY_MS_FOR_CNC_CONTROLLER_IS_READY_CHECK + 1 };

        if(!operatingMode.isState(OperatingState::State::WaitingForCncController))
            return;

        if(elapsedTimeMs < DELAY_MS_FOR_CNC_CONTROLLER_IS_READY_CHECK)
            return;

        gcodeBuffer.setGcode("G91");
        sendBufferedGcode();
        cncSerialBuffer.read();
        handleCncResponse();

        if(gcodeBuffer.isResponseOk())
        {
            operatingMode.switchState(OperatingState::State::Idle);
        }

        elapsedTimeMs = 0;
    }


    void waitForMotionFinished()
    {
        static elapsedMillis elapsedTimeMs{ DELAY_MS_FOR_MOTION_FINISHED_CHECK + 1 };
        static uint8_t pendingResponses{ 0 };

        if(!gcodeBuffer.isProcessed())
            return;

        if(!(operatingMode.isState(OperatingState::State::WaitCommandFromFileMotion) ||
             operatingMode.isState(OperatingState::State::WaitCommandMotion)))
            return;

        if(elapsedTimeMs <= DELAY_MS_FOR_MOTION_FINISHED_CHECK)
            return;
        elapsedTimeMs = 0;

        if(pendingResponses == 0)
        {
            Serial.println("send: '?'");
            cncSerial.write("?");
            pendingResponses++;
        }

        while(cncSerialBuffer.read()) {}

        if(cncSerialBuffer.hasLine())
        {
            if(pendingResponses > 0)
                pendingResponses--;

            const std::string line{ cncSerialBuffer.getLine() };
            Serial.println(std::string("cnc status: '" + line + "'").c_str());
            if(line.rfind("<Idle|", 0) != 0)
                return;
            else
            {
                Serial.println("motion finished");
                pendingResponses = 0;
                // cncSerialBuffer.clear();
            }
        }
        else
        {
            return;
        }

        gcodeBuffer.setMotionFinished();

        if(operatingMode.isState(OperatingState::State::WaitCommandFromFileMotion))
            operatingMode.switchState(OperatingState::State::RunningFromFile);

        if(operatingMode.isState(OperatingState::State::WaitCommandMotion))
            operatingMode.switchState(OperatingState::State::Idle);
    }


    void handleResetWifi()
    {
        if(!operatingMode.isState(OperatingState::State::DoResetWifi))
            return;
        AsyncWiFiManager wifiManager(&webServer, &dnsServer);
        wifiManager.setDebugOutput(false);
        wifiManager.resetSettings();
        EspClass::restart();
    }


    void handleReboot()
    {
        if(!operatingMode.isState(OperatingState::State::DoReboot))
            return;
        EspClass::restart();
    }

    void process()
    {
        cncSerialBuffer.read();
        waitForCncControllerReady();
        handleCncResponse();
        feedGcodeBufferFromFile();
        sendBufferedGcode();
        waitForMotionFinished();

        handleResetWifi();
        handleReboot();
    }
} f;


void setup() { f.setup(); }

void loop() { f.process(); }

#endif
