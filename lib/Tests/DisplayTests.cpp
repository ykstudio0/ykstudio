//-------------------------------------------------------------
// File : DisplayTests.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-21
// Project : SVEMS
// Version : 0.3.0
// Description : 빌드 검증용 Test cpp
//-------------------------------------------------------------

#include <Arduino.h>
#include <cstring>

#include "Tests.h"
#include "DisplayPages.h"
#include "DisplayLayout.h"
#include "DisplayTypes.h"

// Compile-time tests
// static_assert는 빌드 중 검사된다.
// 하나라도 실패하면 펌웨어 빌드가 중단된다.

// DisplayPages constants
static_assert(
    DisplayPages::PAGE_COUNT == 6U,
    "Display page count must be 6");

static_assert(
    DisplayPages::DEFAULT_PAGE == 
        DisplayPages::Page::Overview,
    "Default page must be Overview");

// DisplayLayout constants
static_assert(
    DisplayLayout::SCREEN_WIDTH == 320,
    "Screen width must be 320");

static_assert(
    DisplayLayout::SCREEN_HEIGHT == 240,
    "Screen height must be 240");

static_assert(
    DisplayLayout::HEADER_Y == 0,
    "Header must start at Y=0");

static_assert(
    DisplayLayout::HEADER_WIDTH ==
        DisplayLayout::SCREEN_WIDTH,
    "Header width must match screen width");

static_assert(
    DisplayLayout::FOOTER_WIDTH ==
        DisplayLayout::SCREEN_WIDTH,
    "Footer width must match screen width");

static_assert(
    DisplayLayout::FOOTER_Y + 
        DisplayLayout::FOOTER_HEIGHT == 
        DisplayLayout::SCREEN_HEIGHT,
    "Footer must end at the bottom of the screen");

static_assert(
    DisplayLayout::CONTENT_WIDTH > 0,
    "content width must be positive");

static_assert(
    DisplayLayout::CONTENT_HEIGHT > 0,
    "Content height must be positive");

static_assert(
    DisplayLayout::CONTENT_Y > 
        DisplayLayout::HEADER_DIVIDER_Y,
    "Content must start below the header divider");

static_assert(
    DisplayLayout::FOOTER_DIVIDER_Y <
        DisplayLayout::FOOTER_Y,
    "Footer divider must be above footer");

static_assert(
    DisplayLayout::GetRowY(1) > 
        DisplayLayout::GetRowY(0),
    "Rows must increase dofwnward");

static_assert(
    DisplayLayout::GetRowY(3) < 
        DisplayLayout::FOOTER_DIVIDER_Y,
    "Fourth content row must fit above the footer");

static_assert(
    DisplayLayout::IsValid(),
    "Display layout configuration is invalid");

// DisplayTypes decimal rules

static_assert(
    DisplayTypes::GetDefaultDecimals(
        DisplayTypes::ValueType::Voltage) == 2U,
    "Voltage must use two decimal places");

static_assert(
    DisplayTypes::GetDefaultDecimals(
        DisplayTypes::ValueType::Current) == 2U,
    "Current must use two decimal places");

static_assert(
    DisplayTypes::GetDefaultDecimals(
        DisplayTypes::ValueType::Power) == 1U,
    "Power must use one decimal place");

static_assert(
    DisplayTypes::GetDefaultDecimals(
        DisplayTypes::ValueType::Energy) == 1U,
    "Energy must use one decimal place");

static_assert(
    DisplayTypes::GetDefaultDecimals(
        DisplayTypes::ValueType::Temperature) == 1U,
    "Temperature must use one decimal place");

static_assert(
    DisplayTypes::GetDefaultDecimals(
        DisplayTypes::ValueType::Humidity) == 1U,
    "Humidity must use one decimal place");

static_assert(
    DisplayTypes::GetDefaultDecimals(
        DisplayTypes::ValueType::Percent) == 0U,
    "Percent must use no decimal places");

// DisplayValue default construction
constexpr DisplayTypes::DisplayValue DEFAULT_VALUE;

static_assert(
    DEFAULT_VALUE.value == 0.0f,
    "Default Value must be zero");

static_assert(
    DEFAULT_VALUE.type == 
        DisplayTypes::ValueType::None,
    "Default Value type must be None");

static_assert(
    DEFAULT_VALUE.state ==
        DisplayTypes::WidgetState::Normal,
    "Default widget state must be Normal");

static_assert(
    DEFAULT_VALUE.align ==
        DisplayTypes::TextAlign::Right,
    "Default text alignment must be Right");

static_assert(
    DEFAULT_VALUE.decimals == 1U,
    "Default decimal count must be one");

static_assert(
    DEFAULT_VALUE.visible,
    "Default value must be visible");

static_assert(
    !DEFAULT_VALUE.IsValid(),
    "Default value must be invalid");

static_assert(
    DEFAULT_VALUE.IsVisible(),
    "Default value must be visible");

static_assert(
    !DEFAULT_VALUE.HasAlert(),
    "Default value must not have an alert");

// DisplayValue factory
constexpr DisplayTypes::DisplayValue VOLTAGE_VALUE =
    DisplayTypes::MakeValue(
        14.25f,
        DisplayTypes::ValueType::Voltage);

static_assert(
    VOLTAGE_VALUE.IsValid(),
    "Voltage value must be valid");

static_assert(
    VOLTAGE_VALUE.value == 14.25f,
    "Voltage value was not stored correctly");

static_assert(
    VOLTAGE_VALUE.type ==
        DisplayTypes::ValueType::Voltage,
    "Voltage type was not stored correctly");

static_assert(
    VOLTAGE_VALUE.decimals == 2U,
    "Voltage must use two decimal places");

static_assert(
    VOLTAGE_VALUE.state ==
        DisplayTypes::WidgetState::Normal,
    "Voltage default state must be Normal");

static_assert(
    VOLTAGE_VALUE.align ==
        DisplayTypes::TextAlign::Right,
    "Voltage default alignment must be Right");

 // Alert states
constexpr DisplayTypes::DisplayValue WARNING_VALUE =
    DisplayTypes::MakeValue(
        11.70f,
        DisplayTypes::ValueType::Voltage,
        DisplayTypes::WidgetState::Warning);

static_assert(
    WARNING_VALUE.HasAlert(),
    "Warning state must report an alert");

constexpr DisplayTypes::DisplayValue ALARM_VALUE(
    11.20f,
    DisplayTypes::ValueType::Voltage,
    DisplayTypes::WidgetState::Alarm,
    DisplayTypes::TextAlign::Center,
    2U,
    true);

static_assert(
    ALARM_VALUE.HasAlert(),
    "Alarm state must report an alert");

constexpr DisplayTypes::DisplayValue DISABLED_VALUE(
    0.0f,
    DisplayTypes::ValueType::Power,
    DisplayTypes::WidgetState::Disabled,
    DisplayTypes::TextAlign::Right,
    1U,
    true);

static_assert(
    !DISABLED_VALUE.IsVisible(),
    "Disabled value must not be visible");

constexpr DisplayTypes::DisplayValue HIDDEN_VALUE(
    0.0f,
    DisplayTypes::ValueType::Power,
    DisplayTypes::WidgetState::Normal,
    DisplayTypes::TextAlign::Right,
    1U,
    false);

static_assert(
    !HIDDEN_VALUE.IsVisible(),
    "Hidden value must not be visible");

// Runtime tests
// DisplayPages의 함수들은 현재 inline 함수이며 constexpr가 아니다.
// GetTitle(), GetUnit(), 값 변경 함수도 실행 중 검사한다.
namespace
{
    uint16_t passedCount = 0;
    uint16_t failedCount = 0;

    void Check(
        bool condition,
        const char* testName)
    {
        if (condition)
        {
            passedCount++;

            Serial.print(
                "[PASS] [DISPLAY] ");

            Serial.println(testName);
        }
        else
        {
            failedCount++;

            Serial.print(
                "[FAIL] [DISPLAY] ");

            Serial.println(testName);
        }
    }

    bool StringEquals(
        const char* left,
        const char* right)
    {
        if (left == nullptr ||
            right == nullptr)
        {
            return false;
        }

        return strcmp(left, right) == 0;
    }

    void TestDisplayPages()
    {
        using DisplayPages::Page;

        Check(
            DisplayPages::IsValid(Page::Overview),
            "Overview page is valid");

        Check(
            DisplayPages::IsValid(Page::System),
            "System page is valid");

        Check(
            !DisplayPages::IsValid(Page::Count),
            "Page::Count is not a real page");

        Check(
            DisplayPages::Next(Page::Overview) ==
                Page::Solar,
            "Overview next page is Solar");

        Check(
            DisplayPages::Next(Page::System) ==
                Page::Overview,
            "System next page wraps to Overview");

        Check(
            DisplayPages::Previous(Page::Overview) ==
                Page::System,
            "Overview previous page wraps to System");

        Check(
            DisplayPages::Previous(Page::Battery) ==
                Page::Solar,
            "Battery previous page is Solar");

        Check(
            StringEquals(
                DisplayPages::GetTitle(Page::Overview),
                "OVERVIEW"),
            "Overview title is correct");

        Check(
            StringEquals(
                DisplayPages::GetTitle(Page::Temperature),
                "TEMPERATURE"),
            "Temperature title is correct");

        Check(
            StringEquals(
                DisplayPages::GetTitle(Page::Count),
                "UNKNOWN"),
            "Invalid page title is UNKNOWN");
    }

    void TestDisplayUnits()
    {
        using DisplayTypes::ValueType;

        Check(
            StringEquals(
                DisplayTypes::GetUnit(ValueType::Voltage),
                "V"),
            "Voltage unit is V");

        Check(
            StringEquals(
                DisplayTypes::GetUnit(ValueType::Current),
                "A"),
            "Current unit is A");

        Check(
            StringEquals(
                DisplayTypes::GetUnit(ValueType::Power),
                "W"),
            "Power unit is W");

        Check(
            StringEquals(
                DisplayTypes::GetUnit(ValueType::Energy),
                "Wh"),
            "Energy unit is Wh");

        Check(
            StringEquals(
                DisplayTypes::GetUnit(
                    ValueType::Temperature),
                "C"),
            "Temperature unit is C");

        Check(
            StringEquals(
                DisplayTypes::GetUnit(ValueType::Percent),
                "%"),
            "Percent unit is percent");
        
        Check(
            StringEquals(
                DisplayTypes::GetUnit(ValueType::SignalStrength),
                "dBm"),
            "Signal strength unit is dBm");

        Check(
            StringEquals(
                DisplayTypes::GetUnit(ValueType::None),
                ""),
            "None type has no unit");
    }

    void TestDisplayValueUpdates()
    {
        DisplayTypes::DisplayValue value =
            DisplayTypes::MakeValue(
                12.50f,
                DisplayTypes::ValueType::Voltage);

        value.SetValue(13.25f);

        Check(
            value.value == 13.25f,
            "SetValue updates value");

        value.Set(
            11.80f,
            DisplayTypes::WidgetState::Warning);

        Check(
            value.value == 11.80f,
            "Set updates value");

        Check(
            value.state ==
                DisplayTypes::WidgetState::Warning,
            "Set updates state");

        Check(
            value.HasAlert(),
            "Warning value reports alert");

        value.SetVisible(false);

        Check(
            !value.IsVisible(),
            "SetVisible false hides value");

        value.SetVisible(true);

        Check(
            value.IsVisible(),
            "SetVisible true shows value");
    }
}

namespace Tests
{
    void RunDisplayTests()
    {
        passedCount = 0;
        failedCount = 0;

        Serial.println();
        Serial.println(
            "========== DISPLAY TESTS ==========");

        TestDisplayPages();
        TestDisplayUnits();
        TestDisplayValueUpdates();

        Serial.println(
            "-----------------------------------");

        Serial.print(
            "[DISPLAY TESTS] PASSED: ");

        Serial.println(passedCount);

        Serial.print(
            "[DISPLAY TESTS] FAILED: ");

        Serial.println(failedCount);

        if (failedCount == 0)
        {
            Serial.println(
                "[DISPLAY TESTS] ALL PASSED");
        }
        else
        {
            Serial.println(
                "[DISPLAY TESTS] TEST FAILURE");
        }

        Serial.println(
            "===================================");

        Serial.println();
    }
}
