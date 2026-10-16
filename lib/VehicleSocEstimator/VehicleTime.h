//-------------------------------------------------------------
// File : VehicleTime.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-10-16
// Project : SVEMS
// Version : 0.8.7
// Description : SVEMS Vehicle Time
//-------------------------------------------------------------

#pragma once

#include <stdint.h>
#include "RTCDateTime.h"

class VehicleTime
{
public:
    static bool ToUnixTimestamp(
        const SVEMS::Device::RTCDateTime& time,
        uint32_t& timestamp);
};