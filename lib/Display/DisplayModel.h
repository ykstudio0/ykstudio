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

    struct HeaderData
    {
        static constexpr size_t TIME_TEXT_SIZE = 16U;

        char timeText[TIME_TEXT_SIZE];

        DisplayTypes::DisplayText status;
        DisplayTypes::DisplayText energy;

        constexpr HeaderData()
            : timeText{0},
              status(),
              energy()
        {

        }
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

        DisplayTypes::DisplayText status;

        // BMS Detail
        DisplayTypes::DisplayValue remainingCapacity;

        DisplayTypes::DisplayValue cellVoltage1;
        DisplayTypes::DisplayValue cellVoltage2;
        DisplayTypes::DisplayValue cellVoltage3;
        DisplayTypes::DisplayValue cellVoltage4;

        DisplayTypes::DisplayValue cellDelta;

        DisplayTypes::DisplayValue bmsTemperature;
        DisplayTypes::DisplayValue externalTemperature;

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

            status(),

            remainingCapacity(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::Capacity)),

            cellVoltage1(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::Voltage)),

            cellVoltage2(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::Voltage)),

            cellVoltage3(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::Voltage)),

            cellVoltage4(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::Voltage)),

            cellDelta(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::Voltage)),

            bmsTemperature(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::Temperature)),

            externalTemperature(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::Temperature))
        {
            remainingCapacity.decimals = 1U;

            cellVoltage1.decimals = 3U;
            cellVoltage2.decimals = 3U;
            cellVoltage3.decimals = 3U;
            cellVoltage4.decimals = 3U;

            cellDelta.decimals = 3U;
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
        DisplayTypes::DisplayValue batteryTemperature;
        DisplayTypes::DisplayValue bmsTemperature;
        DisplayTypes::DisplayValue controllerTemperature;

        constexpr TemperatureData()
            : cabinTemperature(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::Temperature)),

              cabinHumidity(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::Humidity)),

              batteryTemperature(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::Temperature)),

              bmsTemperature(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::Temperature)),

              controllerTemperature(
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
        DisplayTypes::DisplayText deviceCount;

        DisplayTypes::DisplayText rs485Status;
        DisplayTypes::DisplayText modbusStatus;
        DisplayTypes::DisplayText deviceManagerStatus;

        DisplayTypes::DisplayText epeverStatus;
        DisplayTypes::DisplayText bmsStatus;

        // SYSTEM DTL(1) - Communication statistics
        DisplayTypes::DisplayValue solarOfflineCount;
        DisplayTypes::DisplayValue chargeOfflineCount;
        DisplayTypes::DisplayValue bmsOfflineCount;
        DisplayTypes::DisplayValue loadOfflineCount;
        DisplayTypes::DisplayValue controllerOfflineCount;
        DisplayTypes::DisplayValue socOfflineCount;

        bool wifiConnected;
        bool rs485Ready;
        bool modbusReady;
        bool deviceManagerReady;
        bool epeverOnline;
        bool bmsOnline;

        constexpr SystemData()
            : currentTime(
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
                    DisplayTypes::ValueType::SignalStrength)),

            deviceCount(),

            rs485Status(),
            modbusStatus(),
            deviceManagerStatus(),

            epeverStatus(),
            bmsStatus(),

            solarOfflineCount(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::None)),

            chargeOfflineCount(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::None)),

            bmsOfflineCount(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::None)),

            loadOfflineCount(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::None)),

            controllerOfflineCount(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::None)),

            socOfflineCount(
                DisplayTypes::MakeValue(
                    0.0f,
                    DisplayTypes::ValueType::None)),

            wifiConnected(false),
            rs485Ready(false),
            modbusReady(false),
            deviceManagerReady(false),
            epeverOnline(false),
            bmsOnline(false)
        {
            currentTime.decimals = 0U;
            uptime.decimals = 0U;
            wifiSignal.decimals = 0U;

            solarOfflineCount.decimals = 0U;
            chargeOfflineCount.decimals = 0U;
            bmsOfflineCount.decimals = 0U;
            loadOfflineCount.decimals = 0U;
            controllerOfflineCount.decimals = 0U;
            socOfflineCount.decimals = 0U;
        }
    };

    // Root display model
    // 모든 페이지 데이터를 하나의 객체에서 관리한다.
    class Model
    {
    public:
        constexpr Model() = default;

        // Header 데이터 접근
        HeaderData& GetHeader()
        {
            return header;
        }

        const HeaderData& GetHeader() const
        {
            return header;
        }

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
            header =
                HeaderData();

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
        HeaderData header;
        OverviewData overview;
        SolarData solar;
        BatteryData battery;
        LoadData load;
        TemperatureData temperature;
        SystemData system;
    };
}