//-------------------------------------------------------------
// File : DisplayLayout.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-20
// Project : SVEMS
// Version : 0.3.0
// Description : Display Layout
//-------------------------------------------------------------

#pragma once

#include "DisplayTypes.h"

namespace Layout
{
    using DisplayTypes::Rect;

    // LCD Size

    constexpr int ScreenWidth  = 240;
    constexpr int ScreenHeight = 320;

    constexpr int Margin = 8;

    // Entire screen
    constexpr Rect Screen
    {
        0,
        0,
        ScreenWidth,
        ScreenHeight
    };

    // Header
    namespace Header
    {
        constexpr Rect Area
        {
            0,
            0,
            ScreenWidth,
            32
        };
        
    }

    // Solar
    namespace Solar
    {
        constexpr Rect Area
        {
            0,
            36,
            ScreenWidth,
            42
        };
    }

    // Battery
    namespace Battery
    {
        constexpr Rect Area
        {
            0,
            80,
            ScreenWidth,
            42
        };
    }

    // Load
    namespace Load
    {
        constexpr Rect Area
        {
            0,
            124,
            ScreenWidth,
            42
        };
    }

    // SOC
    namespace Soc
    {
        constexpr Rect Area
        {
            0,
            168,
            ScreenWidth,
            42
        };
    }

    // Temperature
    namespace Temperature
    {
        constexpr Rect Area
        {
            0,
            212,
            ScreenWidth,
            42
        };
    }

    // Footer
    namespace Footer
    {
        constexpr Rect Area
        {
            0,
            280,
            ScreenWidth,
            40
        };
    }

    // //---------------------------------------------------
    // // Comile-time layout validation
    // //---------------------------------------------------
    // static_assert(ScreenWidth > 0,  "Invalid screen width");
    // static_assert(ScreenHeight > 0, "Invalid screen height");

    // static_assert(Screen.IsValid(), "Invalid screen area");

    // static_assert(Header::Area.IsValid(),      "Invalid header area");
    // static_assert(Solar::Area.IsValid(),       "Invalid solar area");
    // static_assert(Battery::Area.IsValid(),     "Invalid battery area");
    // static_assert(Load::Area.IsValid(),        "Invalid load area");
    // static_assert(Soc::Area.IsValid(),         "Invalid SOC area");
    // static_assert(Temperature::Area.IsValid(), "Invalid temperature area");
    // static_assert(Footer::Area.IsValid(),      "Invalid footer area");

    // static_assert(Header::Area.BottomExclusive() <= Solar::Area.Top(), 
    //             "Header and Solar areas overlap");
                
    // static_assert(Solar::Area.BottomExclusive() <= Battery::Area.Top(),
    //             "Solar and Battery areas overlap");

    // static_assert(Battery::Area.BottomExclusive() <= Load::Area.Top(),
    //             "Battery and Load areas overlap");

    // static_assert(Load::Area.BottomExclusive() <= Soc::Area.Top(),
    //             "Load and SOC areas overlap");

    // static_assert(Soc::Area.BottomExclusive() <= Temperature::Area.Top(),
    //             "SOC and Temperature areas overlap");
                
    // static_assert(Temperature::Area.BottomExclusive() <= Footer::Area.Top(),
    //             "Temperature and Footer areas overlap");

    // static_assert(Footer::Area.RightExclusive() <= ScreenWidth,
    //             "Footer exceeds screen width");

    // static_assert(Footer::Area.BottomExclusive() <= ScreenHeight,
    //             "Footer exceeds screen height");
}

