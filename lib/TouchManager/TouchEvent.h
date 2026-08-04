//-------------------------------------------------------------
// File : TouchEvent.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-04
// Project : SVEMS
// Version : 0.5.6
// Description : Touch input event type
//-------------------------------------------------------------

#pragma once

#include <Arduino.h>

namespace SVEMS::Touch
{
    enum class Event : uint8_t
    {
        None = 0,

        Pressed,
        Released,

        Tap,
        LongPress,

        SwipeLeft,
        SwipeRight,
        SwipeUp,
        SwipeDown
    };
}