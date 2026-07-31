//-------------------------------------------------------------
// File : HeaderWidget.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-24
// Project : SVEMS
// Version : 0.4.0
// Description : Common display header widget
//-------------------------------------------------------------

#pragma once

#include <Arduino.h>

#include "IRenderTarget.h"

namespace DisplayWidgets
{
    class HeaderWidget
    {
    public:
        static void DrawStatic(
            IRenderTarget& target,
            const char* title);

        static void DrawEnergy(
            IRenderTarget& target,
            const char* energyStatusText,
            DisplayTheme::Color energyStatusColor);

        static void DrawTime(
            IRenderTarget& target,
            const char* timeText);

        static void DrawStatus(
            IRenderTarget& target,
            const char* statusText,
            DisplayTheme::Color statusColor);

    private:
        HeaderWidget() = delete;

        // static void ClearDynamic(
        //     IRenderTarget& target);
    };
}