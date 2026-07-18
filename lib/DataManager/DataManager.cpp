//-------------------------------------------------------------
// File : DataManger.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-18
// Project : SVEMS
// Version : 0.2.5
// Descripton : Data 관리
//-------------------------------------------------------------

#include "DataManager.h"

DataManager::SolarData DataManager::Solar;
DataManager::BatteryData DataManager::Battery;
DataManager::LoadData DataManager::Load;
DataManager::TemperatureData DataManager::Temperature;
DataManager::SOCData DataManager::Soc;

void DataManager::ClearUpdates()
{
    Solar.updated = false;
    Battery.updated = false;
    Load.updated = false;
    Temperature.updated = false;
    Soc.updated = false;
}
