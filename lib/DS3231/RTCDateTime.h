//-------------------------------------------------------------
// File : RTCDateTime.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-31
// Project : SVEMS
// Version : 0.5.1
// Description : RTC
//-------------------------------------------------------------

#pragma once

#include <stdint.h>

namespace SVEMS::Device
{
    struct RTCDateTime
    {
        uint16_t year = 2000;

        uint8_t month = 1;
        uint8_t day = 1;
        uint8_t dayOfWeek = 1;

        uint8_t hour = 0;
        uint8_t minute = 0;
        uint8_t second = 0;
    };
}