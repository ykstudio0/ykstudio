//-------------------------------------------------------------
// File : TimeService.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-01
// Project : SVEMS
// Version : 0.5.2
// Description : System Time service
//-------------------------------------------------------------

#include "TimeService.h"

namespace SVEMS::Service
{
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

        if (!Rtc->IsOnline())
        {
            Ready = false;
            return false;
        }

        CurrentTime =
            Rtc->GetTime();

        return true;
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