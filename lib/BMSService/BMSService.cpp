//-------------------------------------------------------------
// File : BMSService.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-06
// Project : SVEMS
// Version : 0.5.7
// Description : PowerBank BMS communication service framework
//-------------------------------------------------------------

#include "BMSService.h"
#include "Pins.h"
#include "Logger.h"
#include "Config.h"

namespace SVEMS::Service
{
    HardwareSerial BMSSerial(2);

    bool BMSService::Ready = false;
    bool BMSService::Receiving = false;

    uint32_t BMSService::LastReceiveTime = 0U;

    // Begin
    bool BMSService::Begin()
    {
        Ready = false;
        Receiving = false;
        LastReceiveTime = 0U;

        BMSSerial.begin(
            115200,
            SERIAL_8N1,
            PIN_BMS_RX,
            PIN_BMS_TX);

        Ready = true;

        Logger::Info("BMS", "Ready (115200bps)");

        return true;
    }

    // Update
    void BMSService::Update()
    {
        if (!Ready)
        {
            return;
        }

        uint8_t buffer[64];
        size_t length = 0U;

        while (BMSSerial.available() > 0 &&
            length < sizeof(buffer))
        {
            buffer[length++] =
                static_cast<uint8_t>(
                    BMSSerial.read());
        }

        if (length == 0U)
        {
            if (Receiving &&
                millis() - LastReceiveTime > 2000U)
            {
                Receiving = false;
            }

            return;
        }

        Receiving = true;
        LastReceiveTime = millis();
        
        if constexpr (ENABLE_BMS_TRACE)
        {
            Logger::Hex("BMS RX", buffer, length);
        }
    }

    // IsReady
    bool BMSService::IsReady()
    {
        return Ready;
    }

    // IsReceiving
    bool BMSService::IsReceiving()
    {
        return Receiving;
    }

    // GetLastReceiveTime
    uint32_t BMSService::GetLastReceiveTime()
    {
        return LastReceiveTime;
    }
}