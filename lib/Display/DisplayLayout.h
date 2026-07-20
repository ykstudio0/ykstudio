//-------------------------------------------------------------
// File : DisplayLayout.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-20
// Project : SVEMS
// Version : 0.3.0
// Description : Display Layout
//-------------------------------------------------------------

#pragma once

namespace Layout
{
    // LCD Size

    constexpr int ScreenWidth  = 240;
    constexpr int ScreenHeight = 320;

    constexpr int Margin = 8;

    // Header
    namespace Header
    {
        constexpr int X = 0;
        constexpr int Y = 0;

        constexpr int Width  = ScreenWidth;
        constexpr int Height = 32;
    }

    // Solar
    namespace Solar
    {
        constexpr int X = 0;
        constexpr int Y = 36;

        constexpr int Width  = ScreenWidth;
        constexpr int Height = 42;
    }

    // Battery
    namespace Battery
    {
        constexpr int X = 0;
        constexpr int Y = 80;

        constexpr int Width  = ScreenWidth;
        constexpr int Height = 42;
    }

    // Load
    namespace Load
    {
        constexpr int X = 0;
        constexpr int Y = 124;

        constexpr int Width  = ScreenWidth;
        constexpr int Height = 42;
    }

    // SOC
    namespace Soc
    {
        constexpr int X = 0;
        constexpr int Y = 168;

        constexpr int Width  = ScreenWidth;
        constexpr int Height = 42;
    }

    // Temperature
    namespace Temperature
    {
        constexpr int X = 0;
        constexpr int Y = 212;

        constexpr int Width = ScreenWidth;
        constexpr int Height = 42;
    }

    // Footer
    namespace Footer
    {
        constexpr int X = 0;
        constexpr int Y = 280;

        constexpr int Width  = ScreenWidth;
        constexpr int Height = 40;
    }
}