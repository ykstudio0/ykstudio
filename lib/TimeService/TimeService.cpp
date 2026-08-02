//-------------------------------------------------------------
// File : TimeService.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-01
// Project : SVEMS
// Version : 0.5.2
// Description : System Time service
//-------------------------------------------------------------

#include <Arduino.h>

#include "TimeService.h"

namespace SVEMS::Service
{
    uint32_t TimeService::LastTickMs = 0U;

    bool TimeService::Ready = false;

    SVEMS::Device::RTCDateTime
        TimeService::CurrentTime;

    SVEMS::Device::DS3231Device*
        TimeService::Rtc = nullptr;

    bool TimeService::Begin(
        SVEMS::Device::DS3231Device& rtc)
    {
        if (!rtc.IsOnline())
        {
            Ready = false;
            return false;
        }
        
        Rtc = &rtc;

        CurrentTime =
            Rtc->GetTime();

        LastTickMs = millis();

        Ready = true;

        return true;
    }

    bool TimeService::Update()
    {
        if (!Ready ||
            Rtc == nullptr)
        {
            return false;
        }

        const uint32_t now = millis();

        const uint32_t elapsedSeconds =
            (now - LastTickMs) / 1000U;

        if (elapsedSeconds == 0U)
        {
            return true;
        }

        LastTickMs +=
            elapsedSeconds * 1000U;

        for (uint32_t i = 0U; i < elapsedSeconds; ++i)
        {
            ++CurrentTime.second;

            if (CurrentTime.second >= 60U)
            {
                CurrentTime.second = 0U;
                ++CurrentTime.minute;
            }

            if (CurrentTime.minute >= 60U)
            {
                CurrentTime.minute = 0U;
                ++CurrentTime.hour;
            }

            if (CurrentTime.hour >= 24U)
            {
                CurrentTime.hour = 0U;
            }
        }

        return true;

        // if (!Rtc->IsOnline())
        // {
        //     Ready = false;
        //     return false;
        // }

        // CurrentTime =
        //     Rtc->GetTime();

        // return true;
    }

    bool TimeService::IsReady()
    {
        return Ready;
    }

    const SVEMS::Device::RTCDateTime&
        TimeService::Now()
    {
        return CurrentTime;
    }
}