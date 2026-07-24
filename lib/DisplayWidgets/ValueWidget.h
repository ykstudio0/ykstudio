//-------------------------------------------------------------
// File : ValueWidget.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-24
// Project : SVEMS
// Version : 0.4.0
// Description : Label, value and unit row widget
//-------------------------------------------------------------

#pragma once

#include <Arduino.h>

#include "IRenderTarget.h"

namespace DisplayWidgets
{
    class ValueWidget
    {
    public:
        static void Draw(
            IRenderTarget& target,
            uint8_t row,
            const char* label,
            const char* valueText,
            const char* unit,
            DisplayTheme::Color valueColor);
    };
}