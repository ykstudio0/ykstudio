//-------------------------------------------------------------
// File : DisplayThemeTests.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-22
// Project : SVEMS
// Version : 0.3.1
// Description : DisplayTheme 검증용 Test cpp
//-------------------------------------------------------------
#include <Arduino.h>
#include "DisplayTheme.h"
#include "Tests.h"

// Compile-time tests
// 상수와 constexpr 함수는 컴파일 단계에서 먼저 검증한다.
// 조건이 하나라도 틀리면 빌드가 중단된다.
static_assert(
    DisplayTheme::IsValid(),
    "DisplayTheme configuration must be valid");

static_assert(
    DisplayTheme::COLOR_BACKGROUND !=
        DisplayTheme::COLOR_TEXT,
    "Background and text colors must be different");

static_assert(
    DisplayTheme::COLOR_WARNING !=
        DisplayTheme::COLOR_ALARM,
    "Warning and alarm colors must be different");

static_assert(
    DisplayTheme::BORDER_WIDTH > 0U,
    "Border width must be greater than zero");

static_assert(
    DisplayTheme::DIVIDER_WIDTH > 0U,
    "Divider width must be greater than zero");

static_assert(
    DisplayTheme::GetValueColor(
        DisplayTypes::WidgetState::Normal) ==
        DisplayTheme::COLOR_VALUE,
    "Normal state must use COLOR_VALUE");

static_assert(
    DisplayTheme::GetValueColor(
        DisplayTypes::WidgetState::Active) ==
        DisplayTheme::COLOR_ACTIVE,
    "Active state must use COLOR_ACTIVE");

static_assert(
    DisplayTheme::GetValueColor(
        DisplayTypes::WidgetState::Warning) ==
        DisplayTheme::COLOR_WARNING,
    "Warning state must use COLOR_WARNING");

static_assert(
    DisplayTheme::GetValueColor(
        DisplayTypes::WidgetState::Alarm) ==
        DisplayTheme::COLOR_ALARM,
    "Alarm state must use COLOR_ALARM");

static_assert(
    DisplayTheme::GetValueColor(
        DisplayTypes::WidgetState::Disabled) ==
        DisplayTheme::COLOR_DISABLED,
    "Disabled state must use COLOR_DISABLED");

static_assert(
    DisplayTheme::GetValueColor(
        DisplayTypes::WidgetState::Unknown) ==
        DisplayTheme::COLOR_UNKNOWN,
    "Unknown state must use COLOR_UNKNOWN");

static_assert(
    DisplayTheme::GetBorderColor(
        DisplayTypes::WidgetState::Normal) ==
        DisplayTheme::COLOR_BORDER,
    "Normal border must use COLOR_BORDER");

static_assert(
    DisplayTheme::GetBorderColor(
        DisplayTypes::WidgetState::Warning) ==
        DisplayTheme::COLOR_WARNING,
    "Warning border must use COLOR_WARNING");

static_assert(
    DisplayTheme::GetBorderColor(
        DisplayTypes::WidgetState::Alarm) ==
        DisplayTheme::COLOR_ALARM,
    "Alarm border must use COLOR_ALARM");

static_assert(
    DisplayTheme::GetFontSize(
        DisplayTheme::FontRole::Small) ==
        DisplayTheme::FONT_SIZE_SMALL,
    "Small font role is incorrect");

static_assert(
    DisplayTheme::GetFontSize(
        DisplayTheme::FontRole::Normal) ==
        DisplayTheme::FONT_SIZE_NORMAL,
    "Normal font role is incorrect");

static_assert(
    DisplayTheme::GetFontSize(
        DisplayTheme::FontRole::Large) ==
        DisplayTheme::FONT_SIZE_LARGE,
    "Large font role is incorrect");

static_assert(
    DisplayTheme::GetFontSize(
        DisplayTheme::FontRole::Title) ==
        DisplayTheme::FONT_SIZE_TITLE,
    "Title font role is incorrect");

static_assert(
    DisplayTheme::GetFontSize(
        DisplayTheme::FontRole::Value) ==
        DisplayTheme::FONT_SIZE_VALUE,
    "Value font role is incorrect");
    
// Runtime test helpers
namespace
{
    uint16_t passedCount = 0U;
    uint16_t failedCount = 0U;

    void Check(
        bool condition,
        const char* description)
    {
        if (condition)
        {
            ++passedCount;

            Serial.print(
                F("[PASS] [DISPLAY THEME] "));

            Serial.println(description);
        }
        else
        {
            ++failedCount;

            Serial.print(
                F("[FAIL] [DISPLAY THEME] "));

            Serial.println(description);
        }
    }

    // Theme configuration tests
    void TestThemeConfiguration()
    {
        Check(
            DisplayTheme::IsValid(),
            "Theme configuration is valid");

        Check(
            DisplayTheme::COLOR_BACKGROUND !=
                DisplayTheme::COLOR_TEXT,
            "Background and text colors are different");

        Check(
            DisplayTheme::COLOR_WARNING !=
                DisplayTheme::COLOR_ALARM,
            "Warning and alarm colors are different");

        Check(
            DisplayTheme::BORDER_WIDTH > 0U,
            "Border width is greater than zero");

        Check(
            DisplayTheme::DIVIDER_WIDTH > 0U,
            "Divider width is greater than zero");

        Check(
            DisplayTheme::CORNER_RADIUS > 0U,
            "Corner radius is greater than zero");

        Check(
            DisplayTheme::PADDING_SMALL <=
                DisplayTheme::PADDING_NORMAL,
            "Small padding is not greater than normal padding");

        Check(
            DisplayTheme::PADDING_NORMAL <=
                DisplayTheme::PADDING_LARGE,
            "Normal padding is not greater than large padding");
    }

    // Value color tests
    void TestValueColors()
    {
        Check(
            DisplayTheme::GetValueColor(
                DisplayTypes::WidgetState::Normal) ==
                DisplayTheme::COLOR_VALUE,
            "Normal state uses value color");

        Check(
            DisplayTheme::GetValueColor(
                DisplayTypes::WidgetState::Active) ==
                DisplayTheme::COLOR_ACTIVE,
            "Active state uses active color");

        Check(
            DisplayTheme::GetValueColor(
                DisplayTypes::WidgetState::Warning) ==
                DisplayTheme::COLOR_WARNING,
            "Warning state uses warning color");

        Check(
            DisplayTheme::GetValueColor(
                DisplayTypes::WidgetState::Alarm) ==
                DisplayTheme::COLOR_ALARM,
            "Alarm state uses alarm color");

        Check(
            DisplayTheme::GetValueColor(
                DisplayTypes::WidgetState::Disabled) ==
                DisplayTheme::COLOR_DISABLED,
            "Disabled state uses disabled color");

        Check(
            DisplayTheme::GetValueColor(
                DisplayTypes::WidgetState::Unknown) ==
                DisplayTheme::COLOR_UNKNOWN,
            "Unknown state uses unknown color");
    }

    // Border color tests
    void TestBorderColors()
    {
        Check(
            DisplayTheme::GetBorderColor(
                DisplayTypes::WidgetState::Normal) ==
                DisplayTheme::COLOR_BORDER,
            "Normal state uses default border color");

        Check(
            DisplayTheme::GetBorderColor(
                DisplayTypes::WidgetState::Active) ==
                DisplayTheme::COLOR_ACTIVE,
            "Active state uses active border color");

        Check(
            DisplayTheme::GetBorderColor(
                DisplayTypes::WidgetState::Warning) ==
                DisplayTheme::COLOR_WARNING,
            "Warning state uses warning border color");

        Check(
            DisplayTheme::GetBorderColor(
                DisplayTypes::WidgetState::Alarm) ==
                DisplayTheme::COLOR_ALARM,
            "Alarm state uses alarm border color");

        Check(
            DisplayTheme::GetBorderColor(
                DisplayTypes::WidgetState::Disabled) ==
                DisplayTheme::COLOR_DISABLED,
            "Disabled state uses disabled border color");

        Check(
            DisplayTheme::GetBorderColor(
                DisplayTypes::WidgetState::Unknown) ==
                DisplayTheme::COLOR_UNKNOWN,
            "Unknown state uses unknown border color");
    }

    // Font role tests
    void TestFontRoles()
    {
        Check(
            DisplayTheme::GetFontSize(
                DisplayTheme::FontRole::Small) ==
                DisplayTheme::FONT_SIZE_SMALL,
            "Small role returns small font size");

        Check(
            DisplayTheme::GetFontSize(
                DisplayTheme::FontRole::Normal) ==
                DisplayTheme::FONT_SIZE_NORMAL,
            "Normal role returns normal font size");

        Check(
            DisplayTheme::GetFontSize(
                DisplayTheme::FontRole::Large) ==
                DisplayTheme::FONT_SIZE_LARGE,
            "Large role returns large font size");

        Check(
            DisplayTheme::GetFontSize(
                DisplayTheme::FontRole::Title) ==
                DisplayTheme::FONT_SIZE_TITLE,
            "Title role returns title font size");

        Check(
            DisplayTheme::GetFontSize(
                DisplayTheme::FontRole::Value) ==
                DisplayTheme::FONT_SIZE_VALUE,
            "Value role returns value font size");
    }
}

// Public test entry point
namespace Tests
{
    void RunDisplayThemeTests()
    {
        passedCount = 0U;
        failedCount = 0U;

        Serial.println();
        Serial.println(
            F("========== DISPLAY THEME TESTS =========="));

        TestThemeConfiguration();
        TestValueColors();
        TestBorderColors();
        TestFontRoles();

        Serial.println(
            F("-----------------------------------------"));

        Serial.print(
            F("[DISPLAY THEME TESTS] PASSED: "));

        Serial.println(passedCount);

        Serial.print(
            F("[DISPLAY THEME TESTS] FAILED: "));

        Serial.println(failedCount);

        if (failedCount == 0U)
        {
            Serial.println(
                F("[DISPLAY THEME TESTS] ALL PASSED"));
        }
        else
        {
            Serial.println(
                F("[DISPLAY THEME TESTS] TEST FAILURE"));
        }

        Serial.println(
            F("========================================="));
    }
}