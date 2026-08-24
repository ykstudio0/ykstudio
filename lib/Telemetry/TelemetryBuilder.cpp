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
#include "DeviceManager.h"

namespace SVEMS::Telemetry
{
    TelemetryState ToTelemetryState(
        DataManager::CommunicationState state)
    {
        switch (state)
        {
            case DataManager::CommunicationState::Online:
                return TelemetryState::Online;

            case DataManager::CommunicationState::Stale:
                return TelemetryState::Stale;

            case DataManager::CommunicationState::Offline:
            default:
                return TelemetryState::Offline;
        }
    }
    
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

        data.solar.dailyEnergy =
            DataManager::Solar.dailyEnergy;

        data.solar.totalEnergy =
            DataManager::Solar.totalEnergy;

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

        data.battery.remainingCapacity =
            DataManager::Battery.remainingCapacity;

        data.battery.totalCapacity =
            DataManager::Battery.totalCapacity;

        data.battery.temperature =
            DataManager::Temperature.battery;

        for (uint8_t i = 0U; i < 4U; ++i)
        {
            data.battery.cellVoltage[i] =
                DataManager::Battery.cellVoltage[i];
        }
        
        data.battery.online =
            DataManager::Battery.status.online;

        // Load
        data.load.voltage =
            DataManager::Load.voltage;

        data.load.current =
            DataManager::Load.current;

        data.load.power =
            DataManager::Load.power;

        data.load.online =
            DataManager::Load.status.online;

        // Temperature
        data.temperature.battery =
            DataManager::Temperature.battery;

        data.temperature.bms =
            DataManager::Temperature.bms;

        data.temperature.powerBankExternal =
            DataManager::Temperature.powerBankExternal;

        data.temperature.controller =
            DataManager::Temperature.controller;

        data.temperature.controllerBoard =
            DataManager::Temperature.controllerBoard;

        data.temperature.cabinState =
            ToTelemetryState(
                DataManager::Temperature.cabinStatus.state
            );

        data.temperature.powerBankState =
            ToTelemetryState(
                DataManager::Temperature.powerBankStatus.state
            );

        data.temperature.controllerState =
            ToTelemetryState(
                DataManager::Temperature.controllerStatus.state
            );
            
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
        data.communication.solarState =
            ToTelemetryState(
                DataManager::Solar.status.state
            );

        data.communication.solarTimeoutCount =
            DataManager::CommStats.solarTimeoutCount;

        data.communication.solarOfflineCount =
            DataManager::CommStats.solarOfflineCount;

        data.communication.chargeState =
            ToTelemetryState(
                DataManager::Charge.status.state
            );

        data.communication.chargeTimeoutCount =
            DataManager::CommStats.chargeTimeoutCount;

        data.communication.chargeOfflineCount =
            DataManager::CommStats.chargeOfflineCount;

        data.communication.modbusNoResponseCount =
            DataManager::CommStats.modbusNoResponseCount;

        data.communication.modbusFrameTooShortCount =
            DataManager::CommStats.modbusFrameTooShortCount;

        data.communication.modbusInvalidSlaveCount =
            DataManager::CommStats.modbusInvalidSlaveCount;

        data.communication.modbusInvalidFunctionCount =
            DataManager::CommStats.modbusInvalidFunctionCount;

        data.communication.modbusInvalidLengthCount =
            DataManager::CommStats.modbusInvalidLengthCount;

        data.communication.modbusCrcErrorCount =
            DataManager::CommStats.modbusCrcErrorCount;

        data.communication.modbusExceptionCount =
            DataManager::CommStats.modbusExceptionCount;

        data.communication.modbusInvalidByteCount =
            DataManager::CommStats.modbusInvalidByteCount;

        data.communication.bmsOfflineCount =
            DataManager::CommStats.batteryOfflineCount;

        data.communication.rs485Ready =
            RS485::IsReady();

        data.communication.modbusReady =
            ModbusRTU::IsReady();

        // System
        data.system.uptimeSeconds =
            millis() / 1000UL;

        data.system.bootCount =
            DataManager::SystemRuntime.bootCount;

        data.system.resetReason =
            DataManager::ResetReasonToString(
                DataManager::SystemRuntime.resetReason
            );

        data.system.wifiConnected =
            SVEMS::Service::WiFiService::IsConnected();

        data.system.wifiRssi =
            SVEMS::Service::WiFiService::GetRssi();

        data.system.deviceCount =
            DeviceManager::GetOnlineDeviceCount();

        data.system.deviceTotal =
            SYSTEM_DEVICE_COUNT;

        data.vehicle.active =
            DataManager::Vehicle.active;
    }
}