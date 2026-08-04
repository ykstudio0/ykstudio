//-------------------------------------------------------------
// File : DisplayRect.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-04
// Project : SVEMS
// Version : 0.5.6
// Description : Common display rectangle type
//-------------------------------------------------------------

#pragma once

#include <stdint.h>

namespace SVEMS::Display
{
    struct Rect
    {
        int16_t x;
        int16_t y;
        int16_t width;
        int16_t height;

        constexpr bool Contains(
            uint16_t pointX,
            uint16_t pointY) const
        {
            return
                pointX >= x &&
                pointX <
                    static_cast<int32_t>(x) +
                    width &&
                pointY >= y &&
                pointY <
                    static_cast<int32_t>(y) +
                    height;
        }
    };
}