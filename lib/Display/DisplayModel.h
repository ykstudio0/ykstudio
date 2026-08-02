//-------------------------------------------------------------
// File : DisplayModel.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-22
// Project : SVEMS
// Version : 0.3.2
// Description : Display data model for all SVEMS pages
//-------------------------------------------------------------

#pragma once

#include <Arduino.h>
#include "DisplayTypes.h"

namespace DisplayModel
{
    enum class EnergyStatus
    {
        Charging,
        Idle,
        Night,
        Warning
    };

    // Overview page data
    // 시스템의 주요 상태를 한 화면에서 요약한다.
    struct OverviewData
    {
        EnergyStatus energyStatus;

        DisplayTypes::DisplayValue solarPower;
        DisplayTypes::DisplayValue batteryVoltage;
        DisplayTypes::DisplayValue batteryPercent;
        DisplayTypes::DisplayValue loadPower;
        DisplayTypes::DisplayValue temperature;
        DisplayTypes::DisplayValue humidity;

        constexpr OverviewData()
            : energyStatus(
                EnergyStatus::Idle),

              solarPower(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::Power)),

              batteryVoltage(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::Voltage)),

              batteryPercent(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::Percent)),
                
              loadPower(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::Power)),

              temperature(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::Temperature)),

              humidity(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::Humidity))
        {

        }
    };

    // Solar page date
    struct SolarData
    {
        DisplayTypes::DisplayValue voltage;
        DisplayTypes::DisplayValue current;
        DisplayTypes::DisplayValue power;
        DisplayTypes::DisplayValue dailyEnergy;
        DisplayTypes::DisplayValue totalEnergy;

        DisplayTypes::DisplayText chargingStage;
        DisplayTypes::DisplayText inputVoltage;
        
        constexpr SolarData()
            : voltage(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::Voltage)),

              current(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::Current)),

              power(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::Power)),

              dailyEnergy(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::Energy)),

              totalEnergy(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::Energy)),

            //   chargingStageText(""),

            //   inputVoltageText("")
            chargingStage(),
            
            inputVoltage()
        {

        }
    };

    // Battery page data
    struct BatteryData
    {
        DisplayTypes::DisplayValue voltage;
        DisplayTypes::DisplayValue current;
        DisplayTypes::DisplayValue power;
        DisplayTypes::DisplayValue percent;
        DisplayTypes::DisplayValue temperature;
        // DisplayTypes::DisplayValue status;

        DisplayTypes::DisplayText status;

        constexpr BatteryData()
            : voltage(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::Voltage)),

              current(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::Current)),

              power(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::Power)),

              percent(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::Percent)),

              temperature(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::Temperature)),

              status()
        {

        }
    };

    // Load page data
    struct LoadData
    {
        DisplayTypes::DisplayValue voltage;
        DisplayTypes::DisplayValue current;
        DisplayTypes::DisplayValue power;
        DisplayTypes::DisplayValue dailyEnergy;

        constexpr LoadData()
            : voltage(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::Voltage)),

              current(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::Current)),

              power(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::Power)),

              dailyEnergy(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::Energy))
        {

        }
    };

    // Temperature page data
    struct TemperatureData
    {
        DisplayTypes::DisplayValue cabinTemperature;
        DisplayTypes::DisplayValue cabinHumidity;
        DisplayTypes::DisplayValue controllerTemperature;
        DisplayTypes::DisplayValue batteryTemperature;

        constexpr TemperatureData()
            : cabinTemperature(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::Temperature)),

              cabinHumidity(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::Humidity)),

              controllerTemperature(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::Temperature)),

              batteryTemperature(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::Temperature))
        {

        }
    };

    // System page data
    struct SystemData
    {
        DisplayTypes::DisplayValue currentTime;
        DisplayTypes::DisplayValue uptime;
        DisplayTypes::DisplayValue heapPercent;
        DisplayTypes::DisplayValue wifiSignal;
        DisplayTypes::DisplayValue deviceCount;

        bool wifiConnected;
        bool rs485Ready;
        bool modbusReady;
        bool deviceManagerReady;

        constexpr SystemData()
            :currentTime(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::Time)),

            uptime(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::Duration)),

            heapPercent(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::Percent)),

            wifiSignal(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::Percent)),

            deviceCount(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::Number)),

            wifiConnected(false),

            rs485Ready(false),

            modbusReady(false),

            deviceManagerReady(false)
        {
            currentTime.decimals = 0U;
            
            deviceCount.type =
                DisplayTypes::ValueType::None;

            deviceCount.decimals = 0U;
        }
    };

    // Root display model
    // 모든 페이지 데이터를 하나의 객체에서 관리한다.
    class Model
    {
    public:
        constexpr Model() = default;

        // 페이지별 데이터 접근
        OverviewData& GetOverview()
        {
            return overview;
        }

        const OverviewData& GetOverview() const
        {
            return overview;
        }

        SolarData& GetSolar()
        {
            return solar;
        }

        const SolarData& GetSolar() const
        {
            return solar;
        }

        BatteryData& GetBattery()
        {
            return battery;
        }

        const BatteryData& GetBattery() const
        {
            return battery;
        }

        LoadData& GetLoad()
        {
            return load;
        }

        const LoadData& GetLoad() const
        {
            return load;
        }

        TemperatureData& GetTemperature()
        {
            return temperature;
        }

        const TemperatureData& GetTemperature() const
        {
            return temperature;
        }

        SystemData& GetSystem()
        {
            return system;
        }

        const SystemData& GetSystem() const
        {
            return system;
        }

        // 모든 값을 초기 상태로 되돌린다.
        void Reset()
        {
            overview =
                OverviewData();

            solar = 
                SolarData();

            battery = 
                BatteryData();
            
            load =
                LoadData();
            
            temperature = 
                TemperatureData();

            system = 
                SystemData();
        }

        // Overview 페이지에 주요 값을 동기화 한다.
        // 원본 페이지 데이터에서 요약 화면 값을 복사한다.
        void SyncOverview()
        {
            overview.solarPower =
                solar.power;

            overview.batteryVoltage =
                battery.voltage;

            overview.batteryPercent =
                battery.percent;

            overview.loadPower =
                load.power;

            overview.temperature =
                temperature.cabinTemperature;

            overview.humidity =
                temperature.cabinHumidity;

            overview.energyStatus =
                DetermineEnergyStatus();
        }
    private:
        EnergyStatus DetermineEnergyStatus() const
        {
            // --------Status Test ----------
            //return EnergyStatus::Warning;
            
            // 가장 높은 우선순위:
            // 주요 에너지 데이터에 Warning 또는 Alarm이 존재하는 경우
            if (solar.power.HasAlert() ||
                battery.voltage.HasAlert() ||
                battery.percent.HasAlert() ||
                load.power.HasAlert())
            {
                return EnergyStatus::Warning;
            }

            if (solar.power.value > 1.6f)
            {
                return EnergyStatus::Charging;
            }

            return EnergyStatus::Idle;
        }
        OverviewData overview;
        SolarData solar;
        BatteryData battery;
        LoadData load;
        TemperatureData temperature;
        SystemData system;
    };
}