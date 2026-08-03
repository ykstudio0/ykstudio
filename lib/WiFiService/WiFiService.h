//-------------------------------------------------------------
// File : WifiService.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-03
// Project : SVEMS
// Version : 0.6.0
// Description : Non-blocking WiFi connection service
//-------------------------------------------------------------

#pragma once

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

    private:
        static void StartConnection();

        static bool Connecting;
        static uint32_t LastAttemptMs;
        static uint32_t ConnectionStartedMs;
    };
}