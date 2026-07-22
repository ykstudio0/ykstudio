//-------------------------------------------------------------
// File : DisplayModelTests.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-22
// Project : SVEMS
// Version : 0.3.2
// Description : Unit tests for DisplayModel
//-------------------------------------------------------------

#include <Arduino.h>

#include "DisplayModel.h"
#include "Tests.h"

// Compile-time tests
// 각 페이지 데이터가 올바른 ValueType으로 초기화되는지
// 컴파일 단계에서 검증한다.
constexpr DisplayModel::OverviewData
    DEFAULT_OVERVIEW_DATA;

constexpr DisplayModel::SolarData   
    DEFAULT_SOLAR_DATA;

constexpr DisplayModel::BatteryData
    DEFAULT_BATTERY_DATA;

constexpr DisplayModel::LoadData
    DEFAULT_LOAD_DATA;

constexpr DisplayModel::TemperatureData
    DEFAULT_TEMPERATURE_DATA;

constexpr DisplayModel::SystemData
    DEFAULT_SYSTEM_DATA;

static_assert(
    DEFAULT_OVERVIEW_DATA.solarPower.type ==
        DisplayTypes::ValueType::Power,
    "Overview solar power type must be Power");

static_assert(
    DEFAULT_OVERVIEW_DATA.batteryVoltage.type ==
        DisplayTypes::ValueType::Voltage,
    "Overview battery voltage type must be Voltage");

static_assert(
    DEFAULT_OVERVIEW_DATA.batteryPercent.type ==
        DisplayTypes::ValueType::Percent,
    "Overview battery percent type must be Percent");

static_assert(
    DEFAULT_SOLAR_DATA.voltage.type ==
        DisplayTypes::ValueType::Voltage,
    "Solar voltage type must be Voltage");

static_assert(
    DEFAULT_SOLAR_DATA.current.type ==
        DisplayTypes::ValueType::Current,
    "Solar current type must be Current");

static_assert(
    DEFAULT_SOLAR_DATA.power.type ==
        DisplayTypes::ValueType::Power,
    "Solar power type must be Power");

static_assert(
    DEFAULT_SOLAR_DATA.dailyEnergy.type ==
        DisplayTypes::ValueType::Energy,
    "Solar daily energy type must be Energy");

static_assert(
    DEFAULT_BATTERY_DATA.percent.type ==
        DisplayTypes::ValueType::Percent,
    "Battery percent type must be Percent");

static_assert(
    DEFAULT_BATTERY_DATA.temperature.type ==
        DisplayTypes::ValueType::Temperature,
    "Battery temperature type must be Temperature");

static_assert(
    DEFAULT_LOAD_DATA.power.type ==
        DisplayTypes::ValueType::Power,
    "Load power type must be Power");

static_assert(
    DEFAULT_TEMPERATURE_DATA.cabinHumidity.type ==
        DisplayTypes::ValueType::Humidity,
    "Cabin humidity type must be Humidity");

static_assert(
    DEFAULT_SYSTEM_DATA.uptime.type ==
        DisplayTypes::ValueType::Duration,
    "System uptime type must be Duration");

static_assert(
    DEFAULT_SYSTEM_DATA.wifiConnected == false,
    "WiFi must be disconnected by default");

static_assert(
    DEFAULT_SYSTEM_DATA.rs485Ready == false,
    "RS485 must not be ready by default");

static_assert(
    DEFAULT_SYSTEM_DATA.modbusReady == false,
    "Modbus must not be ready by default");

static_assert(
    DEFAULT_SYSTEM_DATA.deviceManagerReady == false,
    "DeviceManager must not be ready by default");

// Runtime test helpers
namespace
{
    uint16_t passedCount = 0U;
    uint16_t failedCount = 0U;

    bool IsNearlyEqual(
        float left,
        float right,
        float tolerance = 0.001f)
    {
        const float difference =
            left > right
                ? left - right
                : right - left;

        return difference <= tolerance;
    }

    void Check(
        bool condition,
        const char* description)
    {
        if (condition)
        {
            ++passedCount;

            Serial.print(
                F("[PASS] [DISPLAY MODEL] "));

            Serial.println(description);
        }
        else
        {
            ++failedCount;

            Serial.print(
                F("[FAIL] [DISPLAY MODEL] "));

            Serial.println(description);
        }
    }

    // Default state tests
    void TestDefaultState()
    {
        DisplayModel::Model model;

        Check(
            model.GetSolar().voltage.IsValid(),
            "Solar voltage is valid by default");

        Check(
            model.GetSolar().voltage.type ==
                DisplayTypes::ValueType::Voltage,
            "Solar voltage uses Voltage type");

        Check(
            model.GetBattery().percent.type ==
                DisplayTypes::ValueType::Percent,
            "Battery percent uses Percent type");

        Check(
            model.GetTemperature().cabinHumidity.type ==
                DisplayTypes::ValueType::Humidity,
            "Cabin humidity uses Humidity type");

        Check(
            model.GetSystem().uptime.type ==
                DisplayTypes::ValueType::Duration,
            "System uptime uses Duration type");

        Check(
            IsNearlyEqual(
                model.GetSolar().power.value,
                0.0f),
            "Solar power starts at zero");

        Check(
            IsNearlyEqual(
                model.GetOverview().batteryVoltage.value,
                0.0f),
            "Overview battery voltage starts at zero");

        Check(
            model.GetSystem().wifiConnected == false,
            "WiFi starts disconnected");

        Check(
            model.GetSystem().rs485Ready == false,
            "RS485 starts not ready");

        Check(
            model.GetSystem().modbusReady == false,
            "Modbus starts not ready");

        Check(
            model.GetSystem().deviceManagerReady == false,
            "DeviceManager starts not ready");
    }

    // Page data update tests
    void TestPageDataUpdates()
    {
        DisplayModel::Model model;

        model.GetSolar().voltage.SetValue(
            46.25f);

        Check(
            IsNearlyEqual(
                model.GetSolar().voltage.value,
                46.25f),
            "Solar voltage value can be updated");

        model.GetSolar().current.Set(
            3.48f,
            DisplayTypes::WidgetState::Active);

        Check(
            IsNearlyEqual(
                model.GetSolar().current.value,
                3.48f),
            "Solar current value can be updated");

        Check(
            model.GetSolar().current.state ==
                DisplayTypes::WidgetState::Active,
            "Solar current state can be updated");

        model.GetBattery().voltage.Set(
            11.80f,
            DisplayTypes::WidgetState::Warning);

        Check(
            IsNearlyEqual(
                model.GetBattery().voltage.value,
                11.80f),
            "Battery voltage value can be updated");

        Check(
            model.GetBattery().voltage.state ==
                DisplayTypes::WidgetState::Warning,
            "Battery voltage warning state is stored");

        model.GetBattery().percent.SetValue(
            72.0f);

        Check(
            IsNearlyEqual(
                model.GetBattery().percent.value,
                72.0f),
            "Battery percent can be updated");

        model.GetLoad().power.Set(
            7.5f,
            DisplayTypes::WidgetState::Warning);

        Check(
            IsNearlyEqual(
                model.GetLoad().power.value,
                7.5f),
            "Load power can be updated");

        Check(
            model.GetLoad().power.HasAlert(),
            "Load warning power reports alert");

        model.GetTemperature()
            .cabinTemperature
            .SetValue(27.4f);

        Check(
            IsNearlyEqual(
                model.GetTemperature()
                    .cabinTemperature
                    .value,
                27.4f),
            "Cabin temperature can be updated");

        model.GetSystem().wifiConnected =
            true;

        model.GetSystem().rs485Ready =
            true;

        Check(
            model.GetSystem().wifiConnected,
            "WiFi connection state can be updated");

        Check(
            model.GetSystem().rs485Ready,
            "RS485 ready state can be updated");
    }

    // Overview synchronization tests
    void TestOverviewSynchronization()
    {
        DisplayModel::Model model;

        model.GetSolar().power.Set(
            162.5f,
            DisplayTypes::WidgetState::Active);

        model.GetBattery().voltage.Set(
            13.42f,
            DisplayTypes::WidgetState::Normal);

        model.GetBattery().percent.Set(
            84.0f,
            DisplayTypes::WidgetState::Normal);

        model.GetLoad().power.Set(
            7.2f,
            DisplayTypes::WidgetState::Warning);

        model.GetTemperature()
            .cabinTemperature
            .SetValue(26.8f);

        model.GetTemperature()
            .cabinHumidity
            .SetValue(61.0f);

        model.SyncOverview();

        Check(
            IsNearlyEqual(
                model.GetOverview().solarPower.value,
                162.5f),
            "Overview receives solar power value");

        Check(
            model.GetOverview().solarPower.state ==
                DisplayTypes::WidgetState::Active,
            "Overview receives solar power state");

        Check(
            IsNearlyEqual(
                model.GetOverview()
                    .batteryVoltage
                    .value,
                13.42f),
            "Overview receives battery voltage");

        Check(
            IsNearlyEqual(
                model.GetOverview()
                    .batteryPercent
                    .value,
                84.0f),
            "Overview receives battery percent");

        Check(
            IsNearlyEqual(
                model.GetOverview().loadPower.value,
                7.2f),
            "Overview receives load power");

        Check(
            model.GetOverview().loadPower.state ==
                DisplayTypes::WidgetState::Warning,
            "Overview receives load warning state");

        Check(
            IsNearlyEqual(
                model.GetOverview().temperature.value,
                26.8f),
            "Overview receives cabin temperature");

        Check(
            IsNearlyEqual(
                model.GetOverview().humidity.value,
                61.0f),
            "Overview receives cabin humidity");

        // SyncOverview는 참조가 아니라 값을 복사한다.
        // 원본 데이터가 변경되더라도 다시 동기화하기 전까지
        // Overview 값은 이전 값을 유지해야 한다.

        model.GetSolar().power.SetValue(
            90.0f);

        Check(
            IsNearlyEqual(
                model.GetOverview().solarPower.value,
                162.5f),
            "Overview remains unchanged before resync");

        model.SyncOverview();

        Check(
            IsNearlyEqual(
                model.GetOverview().solarPower.value,
                90.0f),
            "Overview updates after resync");
    }

    // Reset tests
    void TestReset()
    {
        DisplayModel::Model model;

        model.GetSolar().voltage.Set(
            46.0f,
            DisplayTypes::WidgetState::Active);

        model.GetBattery().percent.Set(
            80.0f,
            DisplayTypes::WidgetState::Warning);

        model.GetLoad().power.SetValue(
            12.0f);

        model.GetTemperature()
            .cabinTemperature
            .SetValue(29.0f);

        model.GetSystem().wifiConnected =
            true;

        model.GetSystem().rs485Ready =
            true;

        model.SyncOverview();

        model.Reset();

        Check(
            IsNearlyEqual(
                model.GetSolar().voltage.value,
                0.0f),
            "Reset clears solar voltage");

        Check(
            model.GetSolar().voltage.state ==
                DisplayTypes::WidgetState::Normal,
            "Reset restores solar voltage state");

        Check(
            IsNearlyEqual(
                model.GetBattery().percent.value,
                0.0f),
            "Reset clears battery percent");

        Check(
            model.GetBattery().percent.state ==
                DisplayTypes::WidgetState::Normal,
            "Reset restores battery percent state");

        Check(
            IsNearlyEqual(
                model.GetLoad().power.value,
                0.0f),
            "Reset clears load power");

        Check(
            IsNearlyEqual(
                model.GetTemperature()
                    .cabinTemperature
                    .value,
                0.0f),
            "Reset clears cabin temperature");

        Check(
            model.GetSystem().wifiConnected == false,
            "Reset disconnects WiFi state");

        Check(
            model.GetSystem().rs485Ready == false,
            "Reset clears RS485 ready state");

        Check(
            IsNearlyEqual(
                model.GetOverview().solarPower.value,
                0.0f),
            "Reset clears overview solar power");

        Check(
            model.GetOverview().solarPower.state ==
                DisplayTypes::WidgetState::Normal,
            "Reset restores overview solar state");
    }
}

// Public test entry point
namespace Tests
{
    void RunDisplayModelTests()
    {
        passedCount = 0U;
        failedCount = 0U;

        Serial.println();
        Serial.println(
            F("========== DISPLAY MODEL TESTS =========="));

        TestDefaultState();
        TestPageDataUpdates();
        TestOverviewSynchronization();
        TestReset();

        Serial.println(
            F("-----------------------------------------"));

        Serial.print(
            F("[DISPLAY MODEL TESTS] PASSED: "));

        Serial.println(passedCount);

        Serial.print(
            F("[DISPLAY MODEL TESTS] FAILED: "));

        Serial.println(failedCount);

        if (failedCount == 0U)
        {
            Serial.println(
                F("[DISPLAY MODEL TESTS] ALL PASSED"));
        }
        else
        {
            Serial.println(
                F("[DISPLAY MODEL TESTS] TEST FAILURE"));
        }

        Serial.println(
            F("========================================="));
    }
}