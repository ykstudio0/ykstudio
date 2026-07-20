//-------------------------------------------------------------
// File : DisplayTheme.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-20
// Project : SVEMS
// Version : 0.3.0
// Description : Display color and style definitions
//-------------------------------------------------------------

#pragma once

#include <stdint.h>

namespace Theme
{
    // Color type
    //
    // LCD 연결 전까지는 24-bit RGB 형식으로 관리한다.
    // 실제 출력 시 LovyanGFX 색상 형식으로 변환한다.
    //
    // Format : 0xRRGGBB

    using Color = uint32_t;

    // Colors
    namespace Colors
    {
        // Base
        constexpr Color Background = 0x101418;
        constexpr Color Surface    = 0x1A2026;
        constexpr Color Border     = 0x35404A;

        // Text
        constexpr Color TextPrimary   = 0xF2F5F7;
        constexpr Color TextSecondary = 0xAAB4BC;
        constexpr Color TextDisabled  = 0x66717A;

        // Status
        constexpr Color Online  = 0x35C759;
        constexpr Color Warning = 0xFFB020;
        constexpr Color Error   = 0xFF453A;
        constexpr Color Offline = 0x66717A;

        // Energy categories
        constexpr Color Solar       = 0xFFC928;
        constexpr Color Battery     = 0x35C759;
        constexpr Color Load        = 0x4DA3FF;
        constexpr Color Soc         = 0xA970FF;
        constexpr Color Temperature = 0xFF7043;

        // Header / Footer
        constexpr Color HeaderBackground = 0x182028;
        constexpr Color FooterBackground = 0x182028;

        // Progress / Gauge
        constexpr Color GaugeBackground = 0x2B343C;
        constexpr Color GaugeNormal     = 0x35C759;
        constexpr Color GaugeWarning    = 0xFFB020;
        constexpr Color GaugeCritical   = 0xFF453A;
    }

    namespace Font
    {
        constexpr uint8_t Tiny   = 1;
        constexpr uint8_t Small  = 1;
        constexpr uint8_t Normal = 2;
        constexpr uint8_t Medium = 2;
        constexpr uint8_t Large  = 3;
        constexpr uint8_t Huge   = 4;
    }

    // Line / Border
    namespace Line
    {
        constexpr uint8_t Thin   = 1;
        constexpr uint8_t Normal = 2;
        constexpr uint8_t Thick  = 3;
    }

    // Corner radius
    namespace Radius
    {
        constexpr uint8_t Small  = 3;
        constexpr uint8_t Normal = 6;
        constexpr uint8_t Large  = 10;
    }

    // Spacing
    namespace Spacing
    {
        constexpr uint8_t Tine   = 2;
        constexpr uint8_t Small  = 4;
        constexpr uint8_t Normal = 8;
        constexpr uint8_t Large  = 12;
    }
}