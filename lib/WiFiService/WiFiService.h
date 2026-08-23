//-------------------------------------------------------------
// File : WifiService.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-03
// Project : SVEMS
// Version : 0.5.5
// Description : Non-blocking WiFi connection service
//-------------------------------------------------------------

#pragma once

#include <Arduino.h>
#include <stdint.h>

namespace SVEMS::Service
{
    class WiFiService
    {
    public:
        static void Begin();
        static void Update();

        static bool IsConnected();
        static bool IsConnecting();

        static int32_t GetRssi();

        static bool SaveConfig(
            const String& ssid,
            const String& password
        );

        static bool StartSetupMode();

    private:
        static void StartConnection();

        static bool Connecting;
        static uint32_t LastAttemptMs;
        static uint32_t ConnectionStartedMs;
    };
}