#if !defined(ENV_NATIVE)
#include "Resources.h"
#include <Esp.h>
#include <WString.h>
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
            String line{ cncSerialBuffer.getLine().c_str() };
            if(!gcodeBuffer.isProcessed())
            {
                gcodeBuffer.setResponse(line);
            }
            else
                Serial.println("received unexpected cnc response '" + line + "'");

            if(operatingMode.isState(OperatingState::State::Idle) || operatingMode.isState(OperatingState::State::RunningFromFile))
            {
                display.screen.drawString(0, Display::L3, gcodeBuffer.getResponse());
                display.screen.display();
            }

            static uint8_t subsequentErrors;

            if(!gcodeBuffer.isResponseOk())
            {
                Serial.println("cnc response '" + gcodeBuffer.getResponse() +
                               "' is erroneous (code=" + gcodeBuffer.getErrorCode() + "), retransmit line");
                if(subsequentErrors >= allowedSubsequentErrors)
                {
                    Serial.println(String() + "error: too many erroneous responses (" + subsequentErrors + "), aborting");
                    subsequentErrors = 0;
                    operatingMode.switchState(OperatingState::State::WaitingForCncController);
                }
                subsequentErrors++;
                gcodeBuffer.setTransmitted(false);
                gcodeBuffer.setResponseReceived(false);
            }
            else // response OK
            {
                Serial.println("cnc response '" + gcodeBuffer.getResponse() + "'");
                subsequentErrors = 0;

                if(!gcodeBuffer.isMotionFinished() && operatingMode.isState(OperatingState::State::RunningFromFile))
                {
                    Serial.println("waiting for '" + gcodeBuffer.getGcode() + "' motion to finish");
                    operatingMode.switchState(OperatingState::State::WaitCommandFromFileMotion);
                }
                else if(!gcodeBuffer.isMotionFinished() && operatingMode.isState(OperatingState::State::Idle))
                {
                    Serial.println("waiting for '" + gcodeBuffer.getGcode() + "' motion to finish");
                    operatingMode.switchState(OperatingState::State::WaitCommandMotion);
                }
                else if(operatingMode.isState(OperatingState::State::WaitingForCncController))
                {
                    operatingMode.switchState(OperatingState::State::Idle);
                }
            }

            if(operatingMode.isState(OperatingState::State::RunningFromFile))
            {
                display.screen.drawString(0, Display::L5, String() + "L " + gcodeFileRunner.getCurrentLine());
                display.screen.display();
            }
        }
    }


    void sendBufferedGcode()
    {
        if(!gcodeBuffer.isProcessed() && !gcodeBuffer.isTransmitted())
        {
            Serial.println("send gcode='" + gcodeBuffer.getGcode() + "'");
            if(operatingMode.isState(OperatingState::State::Idle) || operatingMode.isState(OperatingState::State::RunningFromFile))
            {
                display.screen.clear();
                display.screen.drawString(0, Display::L1, "gcode->cnc");
                display.screen.drawString(0, Display::L2, gcodeBuffer.getGcode());
                display.screen.display();
            }
            cncSerial.print(gcodeBuffer.getGcode() + "\n");
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

            const String line{ cncSerialBuffer.getLine().c_str() };
            Serial.println("cnc status: '" + line + "'");
            if(!line.startsWith("<Idle|"))
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
