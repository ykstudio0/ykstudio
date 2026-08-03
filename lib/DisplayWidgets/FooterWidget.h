//-------------------------------------------------------------
// File : FooterWidget.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-24
// Project : SVEMS
// Version : 0.4.0
// Description : Common display Footer widget
//-------------------------------------------------------------

#pragma once

#include <Arduino.h>

#include "IRenderTarget.h"

namespace DisplayWidgets
{
    class FooterWidget
    {
    public:
        static void Draw(
            IRenderTarget& target,
            const char* leftText,
            const char* centerText,
            const char* rightText);
    };
}