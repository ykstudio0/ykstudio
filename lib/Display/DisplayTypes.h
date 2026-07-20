//-------------------------------------------------------------
// File : DisplayTypes.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-20
// Project : SVEMS
// Version : 0.3.0
// Description : Common display geometry types
//-------------------------------------------------------------
 #pragma once

 #include <stdint.h>

 namespace DisplayTypes
 {
    // Point
    struct Point
    {
        int16_t x;
        int16_t y;
    };

    // size
    struct Size
    {
        int16_t width;
        int16_t height;
    };

    // Rectangle
    struct Rect
    {
        int16_t x;
        int16_t y;
        int16_t width;
        int16_t height;

        constexpr int16_t Left() const
        {
            return x;
        }

        constexpr int16_t Top() const
        {
            return y;
        }

        // constexpr int16_t RightExclusive() const
        // {
        //     return static_cast<int16_t>(x + width);
        // }

        // constexpr int16_t BottomExclusive() const
        // {
        //     return static_cast<int16_t>(y + height);
        // }

        constexpr int16_t Right() const
        {
            return x + width;
        }

        constexpr int16_t Bottom() const
        {
            return y + height;
        }

        constexpr Point Position() const
        {
            return { x, y };
        }

        constexpr Size Dimensions() const
        {
            return { width, height };
        }

        // constexpr int16_t Right() const
        // {
        //     return static_cast<int16_t>(x + width - 1);
        // }

        // constexpr int16_t Bottom() const
        // {
        //     return static_cast<int16_t>(y + height - 1);
        // }

        // constexpr Point Position() const
        // {
        //     return Point{ x, y };
        // }

        // constexpr Size Dimensions() const
        // {
        //     return Size{ width, height };
        // }

        // constexpr bool IsValid() const
        // {
        //     return width > 0 && height > 0;
        // }

        // constexpr bool Contains(const Point& point) const 
        // {
        //     return
        //     point.x >= Left() &&
        //     point.x < RightExclusive() &&
        //     point.y >= Top() &&
        //     point.y < BottomExclusive();
        // }
    };
 }