//-------------------------------------------------------------
// File : TelemetryBuilder.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-09
// Project : SVEMS
// Version : 0.6.0
// Description : Builds telemetry snapshot from DataManager
//-------------------------------------------------------------

#include "TelemetryBuilder.h"
#include "DataManager.h"
#include "RS485.h"
#include "ModbusRTU.h"
#include "WiFiService.h"
#include "TimeService.h"


namespace SVEMS::Telemetry
{
    void TelemetryBuilder::Build(
        TelemetryData& data)
    {
        const SVEMS::Device::RTCDateTime& now =
            SVEMS::Service::TimeService::Now();

        data.timestamp.year =
            now.year;

        data.timestamp.month =
            now.month;

        data.timestamp.day =
            now.day;

        data.timestamp.hour =
            now.hour;

        data.timestamp.minute =
            now.minute;

        data.timestamp.second =
            now.second;

        // Solar
        data.solar.voltage =
            DataManager::Solar.voltage;

        data.solar.current =
            DataManager::Solar.current;

        data.solar.power =
            DataManager::Solar.power;

        data.solar.online =
            DataManager::Solar.status.online;

        data.solar.stage =
            DataManager::Charge.stage;

        data.solar.inputVoltage =
            DataManager::Charge.inputVoltage;

        // Battery / BMS
        data.battery.voltage =
            DataManager::Battery.voltage;

        data.battery.current =
            DataManager::Battery.current;

        data.battery.power =
            DataManager::Battery.power;

        data.battery.soc =
            DataManager::Soc.value;

        data.battery.temperature =
            DataManager::Temperature.battery;

        data.battery.online =
            DataManager::Battery.status.online;

        // Environment
        data.environment.cabinTemperature =
            roundf(
                DataManager::Environment.temperature *
                10.0f) /
            10.0f;
            
        data.environment.cabinHumidity =
            roundf(
                DataManager::Environment.humidity *
                10.0f) /
            10.0f;

        data.environment.online =
            DataManager::Environment.status.online;

        // Communication
        data.communication.solarOfflineCount =
            DataManager::CommStats.solarOfflineCount;

        data.communication.chargeOfflineCount =
            DataManager::CommStats.chargeOfflineCount;

        data.communication.bmsOfflineCount =
            DataManager::CommStats.batteryOfflineCount;

        data.communication.rs485Ready =
            RS485::IsReady();

        data.communication.modbusReady =
            ModbusRTU::IsReady();

        // System
        data.system.uptimeSeconds =
            millis() / 1000UL;

        data.system.wifiConnected =
            SVEMS::Service::WiFiService::IsConnected();

        data.system.wifiRssi =
            SVEMS::Service::WiFiService::GetRssi();
    }
}