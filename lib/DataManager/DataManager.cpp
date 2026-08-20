//-------------------------------------------------------------
// File : DataManger.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-18
// Project : SVEMS
// Version : 0.2.5
// Description : Data 관리
//-------------------------------------------------------------

#include "DataManager.h"
#include "Constants.h"

DataManager::SolarData DataManager::Solar;
DataManager::BatteryData DataManager::Battery;
DataManager::ControllerBatteryData DataManager::ControllerBattery;
DataManager::LoadData DataManager::Load;
DataManager::TemperatureData DataManager::Temperature;
DataManager::SocData DataManager::Soc;
DataManager::ChargeData DataManager::Charge;
DataManager::EnvironmentData DataManager::Environment;

DataManager::CommunicationStats DataManager::CommStats;

DataManager::VehicleData DataManager::Vehicle;

void DataManager::ClearUpdates()
{
    Solar.status.updated = false;
    Battery.status.updated = false;
    ControllerBattery.status.updated = false;
    Load.status.updated = false;
    Temperature.powerBankStatus.updated = false;
    Temperature.controllerStatus.updated = false;
    Soc.status.updated = false;
    Charge.status.updated = false;
}

void DataManager::UpdateOnlineStatus(uint32_t now)
{
    // Solar
    const uint32_t solarAge =
        now - Solar.status.lastUpdate;

    if (
        Solar.status.state ==
            CommunicationState::Online &&
        solarAge >
            SOLAR_STALE_TIMEOUT_MS
    )
    {
        Solar.status.state =
            CommunicationState::Stale;

        Solar.status.updated =
            true;
    }

    if (
        Solar.status.state !=
            CommunicationState::Offline &&
        solarAge >
            SOLAR_OFFLINE_TIMEOUT_MS
    )
    {
        Solar.status.state =
            CommunicationState::Offline;

        Solar.status.online =
            false;

        Solar.status.updated =
            true;

        ++CommStats.solarOfflineCount;

        CommStats.solarLastOffline =
            now;
    }

    // Charge

    const uint32_t chargeAge =
        now - Charge.status.lastUpdate;

    if (
        Charge.status.state ==
            CommunicationState::Online &&
        chargeAge >
            CHARGE_STALE_TIMEOUT_MS
    )
    {
        Charge.status.state =
            CommunicationState::Stale;

        Charge.status.updated =
            true;
    }

    if (
        Charge.status.state !=
            CommunicationState::Offline &&
        chargeAge >
            CHARGE_OFFLINE_TIMEOUT_MS
    )
    {
        Charge.status.state =
            CommunicationState::Offline;

        Charge.status.online =
            false;

        Charge.status.updated =
            true;

        ++CommStats.chargeOfflineCount;

        CommStats.chargeLastOffline =
            now;
    }

    if (Battery.status.online &&
        now - Battery.status.lastUpdate >
            BMS_TIMEOUT_MS)
    {
        Battery.status.online = false;
        Battery.status.updated = true;

        ++CommStats.batteryOfflineCount;
        CommStats.batteryLastOffline = now;
    }

    if (ControllerBattery.status.online &&
        now - ControllerBattery.status.lastUpdate >
            CONTROLLER_BATTERY_TIMEOUT_MS)
    {
        ControllerBattery.status.online = false;
        ControllerBattery.status.updated = true;

        ++CommStats.controllerBatteryOfflineCount;
        CommStats.controllerBatteryLastOffline = now;
    }

    if (Load.status.online &&
        now - Load.status.lastUpdate >
            LOAD_TIMEOUT_MS)
    {
        Load.status.online = false;
        Load.status.updated = true;

        ++CommStats.loadOfflineCount;
        CommStats.loadLastOffline = now;
    }

    if (Temperature.powerBankStatus.online &&
        now - Temperature.powerBankStatus.lastUpdate >
            BMS_TIMEOUT_MS)
    {
        Temperature.powerBankStatus.online = false;
        Temperature.powerBankStatus.updated = true;

        ++CommStats.powerBankTemperatureOfflineCount;
        CommStats.powerBankTemperatureLastOffline = now;
    }

    if (Temperature.controllerStatus.online &&
        now - Temperature.controllerStatus.lastUpdate >
            CONTROLLER_TEMPERATURE_TIMEOUT_MS)
    {
        Temperature.controllerStatus.online = false;
        Temperature.controllerStatus.updated = true;

        ++CommStats.controllerTemperatureOfflineCount;
        CommStats.controllerTemperatureLastOffline = now;
    }

    if (Soc.status.online &&
        now - Soc.status.lastUpdate >
            BMS_TIMEOUT_MS)
    {
        Soc.status.online = false;
        Soc.status.updated = true;

        ++CommStats.socOfflineCount;
        CommStats.socLastOffline = now;
    }
}
