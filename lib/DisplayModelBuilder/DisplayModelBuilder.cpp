//-------------------------------------------------------------
// File : DisplayModelBuilder.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-25
// Project : SVEMS
// Version : 0.4.0
// Description : Builds DislayModel from system data
//-------------------------------------------------------------

#include <Arduino.h>

#include "DisplayModelBuilder.h"
#include "DataManager.h"
#include "DisplayTypes.H"

namespace
{
    void ApplyCommunicationState(
        DisplayTypes::DisplayValue& value,
        const DataManager::Status& status)
    {
        // Communication loss is immediately treated as Offline.
        // Recovery is automatic when communication becomes available again.

        if (!status.online)
        {
            value.state =
                DisplayTypes::WidgetState::Offline;

            return;
        }

        if (value.state ==
            DisplayTypes::WidgetState::Offline)
        {
            value.state =
                DisplayTypes::WidgetState::Normal;
        }
    }

    void ApplyAvailabilityState(
        DisplayTypes::DisplayValue& value)
    {
        // Reserved for future device availability policies.
        (void)value;
    }

    void ApplyAlarmState(
        DisplayTypes::DisplayValue& value)
    {
        // Reserved for future device-specific warning/alarm policies.
        (void)value;
    }

    void ApplyStatus(
        DisplayTypes::DisplayValue& value,
        const DataManager::Status& status)
    {
        ApplyCommunicationState(
            value,
            status);
        
        ApplyAvailabilityState(
            value);

        ApplyAlarmState(
            value);
    }

    void BuildSolar(DisplayModel::Model& model)
    {
        DisplayModel::SolarData& solar =
            model.GetSolar();

        solar.voltage =
            DisplayTypes::MakeValue(
                DataManager::Solar.voltage,
                DisplayTypes::ValueType::Voltage);

        solar.current =
            DisplayTypes::MakeValue(
                DataManager::Solar.current,
                DisplayTypes::ValueType::Current);

        solar.power =
            DisplayTypes::MakeValue(
                DataManager::Solar.power,
                DisplayTypes::ValueType::Power);

        solar.chargingStageText = 
            EpeverStatusParser::ToString(
                DataManager::Charge.stage);

        solar.inputVoltageText =
            EpeverStatusParser::ToString(
                DataManager::Charge.inputVoltage);

        ApplyStatus(
            solar.voltage,
            DataManager::Solar.status);

        ApplyStatus(
            solar.current,
            DataManager::Solar.status);

        ApplyStatus(
            solar.power,
            DataManager::Solar.status);

        // DataManager 에 아직 누적 발전량 데이터가 없으므로
        // dailyEnergy와 totalEnergy는 Reset() 기본값을 유지한다.
    }

    void BuildBattery(DisplayModel::Model& model)
    {
        DisplayModel::BatteryData& battery =
            model.GetBattery();

        battery.voltage =
            DisplayTypes::MakeValue(
                DataManager::Battery.voltage,
                DisplayTypes::ValueType::Voltage);

        battery.current =
            DisplayTypes::MakeValue(
                DataManager::Battery.current,
                DisplayTypes::ValueType::Current);

        battery.power =
            DisplayTypes::MakeValue(
                DataManager::Battery.power,
                DisplayTypes::ValueType::Power);

        battery.percent =
            DisplayTypes::MakeValue(
                static_cast<float>(DataManager::Soc.value),
                DisplayTypes::ValueType::Percent);
        
        battery.temperature =
            DisplayTypes::MakeValue(
                DataManager::Temperature.battery,
                DisplayTypes::ValueType::Temperature);

        // battery.statusText =
        //         EpeverStatusParser::ToString(
        //             DataManager::Charge.batteryStatus);

        ApplyStatus(
            battery.voltage,
            DataManager::Battery.status);

        ApplyStatus(
            battery.current,
            DataManager::Battery.status);
        
        ApplyStatus(
            battery.power,
            DataManager::Battery.status);

        ApplyStatus(
            battery.percent,
            DataManager::Battery.status);

        ApplyStatus(
            battery.temperature,
            DataManager::Battery.status);
    }

    void BuildLoad(DisplayModel::Model& model)
    {
        DisplayModel::LoadData& load =
            model.GetLoad();

        load.voltage =
            DisplayTypes::MakeValue(
                DataManager::Load.voltage,
                DisplayTypes::ValueType::Voltage);

        load.current =
            DisplayTypes::MakeValue(
                DataManager::Load.current,
                DisplayTypes::ValueType::Current);

        load.power =
            DisplayTypes::MakeValue(
                DataManager::Load.power,
                DisplayTypes::ValueType::Power);

        ApplyStatus(
            load.voltage,
            DataManager::Load.status);

        ApplyStatus(
            load.current,
            DataManager::Load.status);

        ApplyStatus(
            load.power,
            DataManager::Load.status);

        // DataManager에 아직 일일 소비전력 데이터가 없으므로
        // dailyEnergy는 Reset() 기본값을 유지한다.
    }

    void BuildTemperature(DisplayModel::Model& model)
    {
        DisplayModel::TemperatureData& temperature =
            model.GetTemperature();

        temperature.controllerTemperature =
            DisplayTypes::MakeValue(
                DataManager::Temperature.device,
                DisplayTypes::ValueType::Temperature);

        temperature.batteryTemperature =
            DisplayTypes::MakeValue(
                DataManager::Temperature.battery,
                DisplayTypes::ValueType::Temperature);

        temperature.cabinTemperature =
            DisplayTypes::MakeValue(
                0.0f,
                DisplayTypes::ValueType::Temperature);

        temperature.cabinTemperature.state =
                DisplayTypes::WidgetState::NoData;

        temperature.cabinHumidity =
            DisplayTypes::MakeValue(
                0.0f,
                DisplayTypes::ValueType::Humidity);
                
        temperature.cabinHumidity.state =
                DisplayTypes::WidgetState::NoData;

        ApplyStatus(
            temperature.controllerTemperature,
            DataManager::Temperature.status);

        ApplyStatus(
            temperature.batteryTemperature,
            DataManager::Temperature.status);

        ApplyStatus(
            temperature.cabinTemperature,
            DataManager::Temperature.status);

        ApplyStatus(
            temperature.cabinHumidity,
            DataManager::Temperature.status);
    }

    uint8_t CountOnlineDevices()
    {
        uint8_t count = 0U;

        if (DataManager::Solar.status.online)
        {
            ++count;
        }

        if (DataManager::Battery.status.online)
        {
            ++count;
        }

        if (DataManager::Load.status.online)
        {
            ++count;
        }

        if (DataManager::Temperature.status.online)
        {
            ++count;
        }

        if (DataManager::Soc.status.online)
        {
            ++count;
        }

        return count;
    }

    void BuildSystem(DisplayModel::Model& model)
    {
        DisplayModel::SystemData& system =
            model.GetSystem();

        const float uptimeSeconds =
            static_cast<float>(millis()) / 1000.0f;

        system.uptime =
            DisplayTypes::MakeValue(
                uptimeSeconds,
                DisplayTypes::ValueType::Duration);

        system.deviceCount =
            DisplayTypes::MakeValue(
                static_cast<float>(CountOnlineDevices()),
                DisplayTypes::ValueType::None);

        system.deviceCount.decimals = 0U;

        // 다른 값들은 현재 DataManager에서 제공하지 않는다.
        //
        // system.currentTime
        // system.heapPercent
        // system.wifiSignal
        // system. wifiConnected
        // system.rs485Ready
        // system modbusReady
        // system.deviceManagerReady
        //
        // 따라서 현재는 Reset()의 기본값을 유지한다.
    }
}

namespace DisplayModelBuilder
{
    void Build(DisplayModel::Model& model)
    {
        // 이전 표시 데이터가 남지 않도록 매번 초기화 한다.
        model.Reset();

        BuildSolar(model);
        BuildBattery(model);
        BuildLoad(model);
        BuildTemperature(model);
        BuildSystem(model);

        // Overview는 별도의 데이터를 생성하지 않는다.
        // 상세 페이지에서 생성된 표시 데이터를 요약 화면으로 복사한다.
        model.SyncOverview();
    }
}