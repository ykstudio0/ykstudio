//-------------------------------------------------------------
// File : BMSService.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-06
// Project : SVEMS
// Version : 0.5.7
// Description : PowerBank BMS communication service framework
//-------------------------------------------------------------

#pragma once

#include <Arduino.h>

namespace SVEMS::Service
{
    class BMSService
    {
    public:
        static bool Begin();
        static void Update();

        static bool IsReady();
        static bool IsReceiving();

        static uint32_t GetLastReceiveTime();

    private:
        static bool Ready;
        static bool Receiving;

        static uint32_t LastReceiveTime;
    };
}