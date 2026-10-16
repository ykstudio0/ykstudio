//-------------------------------------------------------------
// File : VehicleTime.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-10-16
// Project : SVEMS
// Version : 0.8.7
// Description : SVEMS Vehicle Time
//-------------------------------------------------------------

#include "VehicleTime.h"

namespace
{
    constexpr int64_t KST_OFFSET_SECONDS = 9LL * 3600LL;

    bool IsLeapYear(uint16_t year)
    {
        return (year % 400U == 0U) ||
               ((year % 4U == 0U) &&
                (year % 100U != 0U));
    }

    uint8_t DaysInMonth(uint16_t year, uint8_t month)
    {
        static const uint8_t days[12] =
        {
            31, 28, 31, 30, 31, 30,
            31, 31, 30, 31, 30, 31
        };

        if (month < 1 || month > 12)
            return 0;

        if (month == 2 && IsLeapYear(year))
            return 29;

        return days[month - 1];
    }
}

bool VehicleTime::ToUnixTimestamp(
    const SVEMS::Device::RTCDateTime& time,
    uint32_t& timestamp)
{
    if (time.year < 2020 ||
        time.year > 2099 ||
        time.month < 1 ||
        time.month > 12 ||
        time.day < 1 ||
        time.day > DaysInMonth(time.year, time.month) ||
        time.hour > 23 ||
        time.minute > 59 ||
        time.second > 59)
    {
        return false;
    }

    uint32_t days = 0;

    for (uint16_t year = 1970;
         year < time.year;
         ++year)
    {
        days += IsLeapYear(year) ? 366U : 365U;
    }

    for (uint8_t month = 1;
         month < time.month;
         ++month)
    {
        days += DaysInMonth(time.year, month);
    }

    days += time.day - 1U;

    const int64_t unixSeconds =
        static_cast<int64_t>(days) * 86400LL +
        static_cast<int64_t>(time.hour) * 3600LL +
        static_cast<int64_t>(time.minute) * 60LL +
        time.second -
        KST_OFFSET_SECONDS;

    if (unixSeconds < 0 ||
        unixSeconds > UINT32_MAX)
    {
        return false;
    }

    timestamp = static_cast<uint32_t>(unixSeconds);

    return true;
}