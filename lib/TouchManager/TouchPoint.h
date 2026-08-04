//-------------------------------------------------------------
// File : TouchPoint.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-04
// Project : SVEMS
// Version : 0.5.6
// Description : Touch screen coordinate
//-------------------------------------------------------------

#pragma once

#include <Arduino.h>

namespace SVEMS::Touch
{
    struct TouchPoint
    {
        uint16_t x = 0U;
        uint16_t y = 0U;
    };
}