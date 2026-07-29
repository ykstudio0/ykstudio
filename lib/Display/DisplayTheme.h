//-------------------------------------------------------------
// File : DisplayTheme.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-20
// Project : SVEMS
// Version : 0.3.0
// Description : Display color and style definitions
//-------------------------------------------------------------

#pragma once

#include <Arduino.h>
// #include <stdint.h>
#include "DisplayTypes.h"

namespace DisplayTheme
{
    // Color type
    //
    // 대부분의 ESP32 TFT LCD에서 사용하는 RGB565 형식이다.
    // 실제 출력 시 LovyanGFX 색상 형식으로 변환한다.
    //
    // Format : 0xRRGGBB

    using Color = uint16_t;

    constexpr Color BLACK      = 0x0000;
    constexpr Color WHITE      = 0xFFFF;
    
    constexpr Color RED        = 0xF800;
    constexpr Color GREEN      = 0x07E0;
    constexpr Color BLUE       = 0x001F;

    constexpr Color YELLOW     = 0xFFE0;
    constexpr Color ORANGE     = 0xFD20;
    constexpr Color CYAN       = 0x07FF;
    constexpr Color MAGENTA    = 0xF81F;

    constexpr Color DARK_GRAY  = 0x4208;
    constexpr Color GRAY       = 0x8410;
    constexpr Color LIGHT_GRAY = 0xC618;

    //Semantic colors
    //
    // Renderer에서는 가능하면 기본 색상보다
    // 아래의 의미 기반 색상을 사용한다.
    constexpr Color COLOR_BACKGROUND =
        BLACK;

    constexpr Color COLOR_HEADER_BACKGROUND =
        0x1082;

    constexpr Color COLOR_FOOTER_BACKGROUND = 
        0x1082;

    constexpr Color COLOR_PANEL_BACKGROUND =
        0X0841;

    constexpr Color COLOR_TITLE =
        // WHITE;
        LIGHT_GRAY;

    constexpr Color COLOR_TEXT = 
        WHITE;

    constexpr Color COLOR_LABEL =
        LIGHT_GRAY;

    constexpr Color COLOR_VALUE =
        WHITE;

    constexpr Color COLOR_UNIT =
        GRAY;

    constexpr Color COLOR_DIVIDER =
        DARK_GRAY;

    constexpr Color COLOR_BORDER =
        DARK_GRAY;

    constexpr Color COLOR_ACTIVE =
        GREEN;

    constexpr Color COLOR_SUCCESS =
        GREEN;

    constexpr Color COLOR_INFO =
        BLUE;

    constexpr Color COLOR_WARNING =
        ORANGE;

    constexpr Color COLOR_ALARM =
        RED;

    constexpr Color COLOR_DISABLED =
        GRAY;

    constexpr Color COLOR_UNKNOWN =
        MAGENTA;

    // Font roles
    // 실제 TFT 폰트 번호나 FreeFont 객체는
    // 이후 DisplayRenderer에서 연결한다.
    enum class FontRole : uint8_t
    {
        Small = 0,
        Normal,
        Large,
        Title,
        Value
    };

    // Text sizes
    // 기본 내장 폰트를 사용하는 경우의 크기 배율이다.
    // 실제 LCD 적용 시 조정할 수 있다.
    constexpr uint8_t FONT_SIZE_SMALL  = 1U;
    constexpr uint8_t FONT_SIZE_NORMAL = 1U;
    constexpr uint8_t FONT_SIZE_LARGE  = 2U;
    constexpr uint8_t FONT_SIZE_TITLE  = 2U;
    constexpr uint8_t FONT_SIZE_VALUE  = 2U;

    // Shape and spacing
    constexpr uint8_t BORDER_WIDTH =
        1U;

    constexpr uint8_t DIVIDER_WIDTH =
        1U;

    constexpr uint8_t CORNER_RADIUS =
        4U;

    constexpr uint8_t PADDING_SMALL =
        2U;

    constexpr uint8_t PADDING_NORMAL =
        4U;

    constexpr uint8_t PADDING_LARGE =
        8U;

    constexpr uint8_t TEXT_SPACING =
        4U;

    // State color helpers
    // C++11 constexpr 규칙에 맞도록
    // 하나의 return 표현식으로 구성한다.
    constexpr Color GetValueColor(
        DisplayTypes::WidgetState state)
    {
        return
            state == DisplayTypes::WidgetState::Active
                ? COLOR_ACTIVE
                :
            state == DisplayTypes::WidgetState::Warning
                ? COLOR_WARNING
                :
            state == DisplayTypes::WidgetState::Alarm
                ? COLOR_ALARM
                :
            state == DisplayTypes::WidgetState::Disabled
                ? COLOR_DISABLED
                :
            state == DisplayTypes::WidgetState::NoData
                ? COLOR_DISABLED
                :
            state == DisplayTypes::WidgetState::Offline
                ? COLOR_DISABLED
                :
                COLOR_VALUE;
    }

    // 상태별 테두리 색상
    constexpr Color GetBorderColor(
        DisplayTypes::WidgetState state)
    {
        return
            state == DisplayTypes::WidgetState::Active
                ? COLOR_ACTIVE
                :
            state == DisplayTypes::WidgetState::Warning
                ? COLOR_WARNING
                :
            state == DisplayTypes::WidgetState::Alarm
                ? COLOR_ALARM
                :
            state == DisplayTypes::WidgetState::Disabled
                ? COLOR_DISABLED
                :
            state == DisplayTypes::WidgetState::NoData
                ? COLOR_DISABLED
                :
            state == DisplayTypes::WidgetState::Offline
                ? COLOR_DISABLED
                :
                COLOR_BORDER;
    }

    // FontRole을 기본 글자 크기로 변환한다.
    constexpr uint8_t GetFontSize(
        FontRole role)
    {
        return
            role == FontRole::Small
                ? FONT_SIZE_SMALL
                :
            role == FontRole::Large
                ? FONT_SIZE_LARGE
                :
            role == FontRole::Title
                ? FONT_SIZE_TITLE
                :
            role == FontRole::Value
                ? FONT_SIZE_VALUE
                :
                FONT_SIZE_NORMAL;
    }

    constexpr bool IsValid()
    {
        return
            BORDER_WIDTH > 0U &&
            DIVIDER_WIDTH > 0U &&
            FONT_SIZE_SMALL > 0U &&
            FONT_SIZE_NORMAL > 0U &&
            FONT_SIZE_LARGE > 0U &&
            FONT_SIZE_TITLE > 0U &&
            FONT_SIZE_VALUE > 0U &&
            COLOR_BACKGROUND != COLOR_TEXT &&
            COLOR_WARNING != COLOR_ALARM;
    }
}