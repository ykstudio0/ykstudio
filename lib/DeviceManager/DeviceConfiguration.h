//-------------------------------------------------------------
// File : DeviceConfiguration.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-09-01
// Project : SVEMS
// Version : 0.7.2
// Description : Device Configuration
//-------------------------------------------------------------

#pragma once

namespace SVEMS::Device
{
    struct DeviceConfiguration
    {
        bool mppt = true;
        bool bms = true;
        bool sht40 = true;
        bool rtc = true;
    };
}