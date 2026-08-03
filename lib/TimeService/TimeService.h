//-------------------------------------------------------------
// File : TimeService.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-01
// Project : SVEMS
// Version : 0.5.2
// Description : System time service
//-------------------------------------------------------------

#pragma once

#include <stdint.h>

#include "RTCDateTime.h"
#include "DS3231Device.h"

namespace SVEMS::Service
{
    class TimeService
    {
    public:
        static bool Begin(
            SVEMS::Device::DS3231Device& rtc);

        static bool Update();

        static bool Synchronize(
            const SVEMS::Device::RTCDateTime& time);
            
        static bool IsReady();

        static const SVEMS::Device::RTCDateTime& Now();

    private:
        static bool Ready;

        static SVEMS::Device::RTCDateTime CurrentTime;

        static SVEMS::Device::DS3231Device* Rtc;

        static uint32_t LastTickMs;
    };
}