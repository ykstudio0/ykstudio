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
            IncrementOneSecond();
        }

        return true;
    }

    bool TimeService::Synchronize(
        const SVEMS::Device::RTCDateTime& time)
    {
        if (!Ready || Rtc == nullptr)
        {
            return false;
        }

        if (!Rtc->SetTime(time))
        {
            return false;
        }

        CurrentTime = 
            time;

        LastTickMs =
            millis();

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

    bool TimeService::IsLeapYear(
        uint16_t year)
    {
        if ((year % 400U) == 0U)
        {
            return true;
        }

        if ((year % 100U) == 0U)
        {
            return false;
        }

        return
            (year % 4U) == 0U;
    }

    uint8_t TimeService::GetDaysInMonth(
        uint16_t year,
        uint8_t month)
    {
        switch (month)
        {
            case 1u:
            case 3U:
            case 5U:
            case 7U:
            case 8U:
            case 10U:
            case 12U:
                return 31U;
            case 4U:
            case 6U:
            case 9U:
            case 11U:
                return 30U;
            case 2U:
                return
                    IsLeapYear(year)
                        ? 29U
                        : 28U;
            default:
                return 31U;
        }
    }

    void TimeService::IncrementOneSecond()
    {
        ++CurrentTime.second;

        if (CurrentTime.second < 60U)
        {
            return;
        }

        CurrentTime.second = 0U;
        ++CurrentTime.minute;

        if (CurrentTime.minute < 60U)
        {
            return;
        }

        CurrentTime.minute = 0U;
        ++CurrentTime.hour;

        if (CurrentTime.hour < 24U)
        {
            return;
        }

        CurrentTime.hour = 0U;

        ++CurrentTime.dayOfWeek;

        if (CurrentTime.dayOfWeek > 7U)
        {
            CurrentTime.dayOfWeek = 1U;
        }

        ++CurrentTime.day;

        const uint8_t daysInMonth =
            GetDaysInMonth(
                CurrentTime.year,
                CurrentTime.month);

        if (CurrentTime.day <= daysInMonth)
        {
            return;
        }

        CurrentTime.day = 1U;
        ++CurrentTime.month;

        if (CurrentTime.month <= 12U)
        {
            return;
        }

        CurrentTime.month = 1U;
        ++CurrentTime.year;
    }
}