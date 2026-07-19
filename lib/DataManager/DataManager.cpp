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
DataManager::LoadData DataManager::Load;
DataManager::TemperatureData DataManager::Temperature;
DataManager::SocData DataManager::Soc;

void DataManager::ClearUpdates()
{
    Solar.status.updated = false;
    Battery.status.updated = false;
    Load.status.updated = false;
    Temperature.status.updated = false;
    Soc.status.updated = false;
}

void DataManager::UpdateOnlineStatus(uint32_t now)
{
    

    if (Solar.status.online && 
            now - Solar.status.lastUpdate > SOLAR_TIMEOUT_MS)
    {
        Solar.status.online = false;
    }

    if (Battery.status.online && 
            now - Battery.status.lastUpdate > BATTERY_TIMEOUT_MS)
    {
        Battery.status.online = false;
    }

    if (Load.status.online && 
            now - Load.status.lastUpdate > LOAD_TIMEOUT_MS)
    {
        Load.status.online = false;
    }

    if (Temperature.status.online && 
            now - Temperature.status.lastUpdate > TEMPERATURE_TIMEOUT_MS)
    {
        Temperature.status.online = false;
    }

    if (Soc.status.online && 
            now - Soc.status.lastUpdate > SOC_TIMEOUT_MS)
    {
        Soc.status.online = false;
    }
}