//-------------------------------------------------------------
// File : DisplayModelBuilder.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-25
// Project : SVEMS
// Version : 0.4.0
// Description : Builds DislayModel from system data
//-------------------------------------------------------------

#include <Arduino.h>
#include <WiFi.h>

#include "DisplayModelBuilder.h"
#include "DataManager.h"
#include "DisplayTypes.h"
#include "DisplayTheme.h"
#include "TimeService.h"
#include "EpeverStatusParser.h"
#include "Logger.h"
#include "WiFiService.h"
#include "RS485.h"
#include "ModbusRTU.h"
#include "DeviceManager.h"

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

        solar.chargingStage.text = 
            EpeverStatusParser::ToString(
                DataManager::Charge.stage);

        solar.chargingStage.color = DisplayTheme::COLOR_VALUE;

        solar.inputVoltage.text =
            EpeverStatusParser::ToString(
                DataManager::Charge.inputVoltage);
            
        solar.inputVoltage.color = DisplayTheme::COLOR_VALUE;

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

        // 배터리 상태는 향후 파워뱅크 적산계/BMS 데이터로 구성한다.
        // EPEVER 충전 컨트롤러의 배터리 상태는 의미가 다르므로 사용하지 않는다.
        // Do not use the EPEVER controller battery status here.
        // battery.status.text =
        //         EpeverStatusParser::ToString(
        //             DataManager::Charge.batteryStatus);

        // battery.status.color = DisplayTheme::COLOR_VALUE;

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
            DataManager::Soc.status);

        ApplyStatus(
            battery.temperature,
            DataManager::Temperature.powerBankStatus);
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

        temperature.cabinTemperature =
            DisplayTypes::MakeValue(
                DataManager::Environment.temperature,
                DisplayTypes::ValueType::Temperature);

        temperature.cabinHumidity =
            DisplayTypes::MakeValue(
                DataManager::Environment.humidity,
                DisplayTypes::ValueType::Humidity);

        temperature.batteryTemperature =
            DisplayTypes::MakeValue(
                DataManager::Temperature.battery,
                DisplayTypes::ValueType::Temperature);

        temperature.bmsTemperature =
            DisplayTypes::MakeValue(
                DataManager::Temperature.bms,
                DisplayTypes::ValueType::Temperature);

        temperature.controllerTemperature =
            DisplayTypes::MakeValue(
                DataManager::Temperature.controllerBoard,
                DisplayTypes::ValueType::Temperature);

        ApplyStatus(
            temperature.cabinTemperature,
            DataManager::Environment.status);

        ApplyStatus(
            temperature.batteryTemperature,
            DataManager::Temperature.powerBankStatus);

        ApplyStatus(
            temperature.bmsTemperature,
            DataManager::Temperature.powerBankStatus);

        ApplyStatus(
            temperature.controllerTemperature,
            DataManager::Temperature.controllerStatus);
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

        if (DataManager::Temperature.cabinStatus.online ||
            DataManager::Temperature.powerBankStatus.online ||
            DataManager::Temperature.controllerStatus.online)
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

        const SVEMS::Device::RTCDateTime& now =
            SVEMS::Service::TimeService::Now();

        const uint32_t currentTimeSeconds =
            static_cast<uint32_t>(now.hour) * 3600UL +
            static_cast<uint32_t>(now.minute) * 60UL +
            static_cast<uint32_t>(now.second);

        system.currentTime =
            DisplayTypes::MakeValue(
                static_cast<float>(currentTimeSeconds),
                DisplayTypes::ValueType::Time);

        system.currentTime.decimals = 0U;

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

        // WiFi RSSI
        system.wifiConnected =
            SVEMS::Service::WiFiService::IsConnected();

        system.wifiSignal =
            DisplayTypes::MakeValue(
                system.wifiConnected
                    ? static_cast<float>(WiFi.RSSI())
                    : 0.0f,
                DisplayTypes::ValueType::SignalStrength);

        system.wifiSignal.decimals = 0U;

        if (!system.wifiConnected)
        {
            system.wifiSignal.state =
                DisplayTypes::WidgetState::Offline;
        }

        // Heap Usage
        const uint32_t totalHeap =
            ESP.getHeapSize();

        const uint32_t freeHeap =
            ESP.getFreeHeap();

        float heapUsedPercent = 0.0f;

        if (totalHeap > 0U)
        {
            heapUsedPercent =
                static_cast<float>(
                    totalHeap - freeHeap) *
                100.0f /
                static_cast<float>(
                    totalHeap);
        }

        system.heapPercent =
            DisplayTypes::MakeValue(
                heapUsedPercent,
                DisplayTypes::ValueType::Percent);

        system.heapPercent.decimals = 0U;

        // Communication / Manager State
        system.rs485Ready =
            RS485::IsReady();

        system.modbusReady =
            ModbusRTU::IsReady();

        system.deviceManagerReady =
            DeviceManager::IsReady();

        // Status Text
        system.rs485Status.text =
            system.rs485Ready
                ? "OK"
                : "FAIL";

        system.rs485Status.color =
            system.rs485Ready
                ? DisplayTheme::COLOR_VALUE
                : DisplayTheme::COLOR_ALARM;

        system.modbusStatus.text =
            system.modbusReady
                ? "OK"
                : "FAIL";

        system.modbusStatus.color =
            system.modbusReady
                ? DisplayTheme::COLOR_VALUE
                : DisplayTheme::COLOR_ALARM;

        system.deviceManagerStatus.text =
            system.deviceManagerReady
                ? "OK"
                : "FAIL";

        system.deviceManagerStatus.color =
            system.deviceManagerReady
                ? DisplayTheme::COLOR_VALUE
                : DisplayTheme::COLOR_ALARM;
    }

    void BuildHeader(DisplayModel::Model& model)
    {
        DisplayModel::HeaderData& header =
            model.GetHeader();

        const DisplayModel::SystemData& system =
            model.GetSystem();

        const DisplayModel::OverviewData& overview =
            model.GetOverview();

        // Time
        const uint32_t totalSeconds =
            system.currentTime.value > 0.0f
                ? static_cast<uint32_t>(
                    system.currentTime.value)
                : 0U;
                
        const uint32_t normalizedSeconds =
            totalSeconds % 86400UL;

        const uint32_t hours = 
            normalizedSeconds / 3600UL;

        const uint32_t minutes =
            (normalizedSeconds % 3600UL) / 60UL;

        const uint32_t seconds =
            normalizedSeconds % 60UL;

        const SVEMS::Device::RTCDateTime& now =
            SVEMS::Service::TimeService::Now();
            
        snprintf(
            header.timeText,
            sizeof(header.timeText),
            "%02u:%02u:%02u",
            static_cast<unsigned>(hours),
            static_cast<unsigned>(minutes),
            static_cast<unsigned>(seconds));

        //---------------------------------------------------------
        // System status
        //---------------------------------------------------------

        if (!system.wifiConnected)
        {
            header.status.text = "NET";
            header.status.color =
                DisplayTheme::COLOR_WARNING;
        }
        else if (!system.rs485Ready)
        {
            header.status.text = "485";
            header.status.color =
                DisplayTheme::COLOR_WARNING;
        }
        else if (!system.modbusReady)
        {
            header.status.text = "MOD";
            header.status.color =
                DisplayTheme::COLOR_WARNING;
        }
        else if (!system.deviceManagerReady)
        {
            header.status.text = "DEV";
            header.status.color =
                DisplayTheme::COLOR_WARNING;
        }
        else
        {
            header.status.text = "OK";
            header.status.color =
                DisplayTheme::COLOR_ACTIVE;
        }

        //---------------------------------------------------------
        // Energy status
        //---------------------------------------------------------

        switch (overview.energyStatus)
        {
            case DisplayModel::EnergyStatus::Charging:
                header.energy.text = "Charging";
                header.energy.color =
                    DisplayTheme::COLOR_SUCCESS;
                break;

            case DisplayModel::EnergyStatus::Idle:
                header.energy.text = "Idle";
                header.energy.color =
                    DisplayTheme::COLOR_INFO;
                break;

            case DisplayModel::EnergyStatus::Night:
                header.energy.text = "Night";
                header.energy.color =
                    DisplayTheme::COLOR_LABEL;
                break;

            case DisplayModel::EnergyStatus::Warning:
                header.energy.text = "Warning";
                header.energy.color =
                    DisplayTheme::COLOR_ALARM;
                break;

            default:
                header.energy.text = "---";
                header.energy.color =
                    DisplayTheme::COLOR_INFO;
                break;
        }
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

        // System과 Overview가 완성된 후
        // Header 표시 데이터를 구성한다.
        BuildHeader(model);
    }
}