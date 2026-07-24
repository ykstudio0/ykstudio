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
        static void Draw(
            IRenderTarget& target,
            const char* title,
            const char* timeText,
            const char* statusText,
            DisplayTheme::Color statusColor);
    };
}