//-------------------------------------------------------------
// File : Epever.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-08
// Project : SVEMS
// Version : 0.1.6
// Description : Epever 장치
//-------------------------------------------------------------

#include "Epever.h"
#include "Logger.h"
#include "EpeverRegisters.h"
#include "Modbus.h"
#include "ModbusRTU.h"
#include "Config.h"
#include "Units.h"
#include "EpeverMaps.h"
#include "DataManager.h"

EpeverData Epever::Data;

bool Epever::Begin()
{
    return true;
}

bool Epever::Update()
{
    bool ok = true;

    return ok;
}

bool Epever::ReadSolar()
{
    EpeverMap::Solar solar;

    if (!ReadRegisters(
        EpeverRegister::PV_ARRAY_VOLTAGE,
        sizeof(solar) / sizeof(uint16_t),
        (uint16_t*)&solar))
    {
        DataManager::Solar.status.updated = false;
        DataManager::Solar.status.online = false;
        return false;
    }

    // PV Voltage
    DataManager::Solar.voltage = ToVoltage(solar.voltage);

    // PV Current
    DataManager::Solar.current = ToCurrent(solar.current);

    // PV Power (32bit)
    uint32_t rawPower =
        ((uint32_t)solar.powerHigh << 16) |
        solar.powerLow;
    
    DataManager::Solar.power = ToPower(rawPower);

    // Solar Status
    DataManager::Solar.status.updated = true;
    // DataManager::Solar.status.online = true;
    DataManager::Solar.status.lastUpdate = millis();

    return true;
}

// Read Battery Information
bool Epever::ReadBattery()
{
    EpeverMap::Battery battery;
    
    // Read Battery Voltage / Current
    if (!ReadRegisters(
        EpeverRegister::EPEVER_BATTERY_VOLTAGE,
        sizeof(battery) / sizeof(uint16_t),
        (uint16_t*)&battery))
    {
        DataManager::Battery.status.updated = false;
        return false;
    }
    
    // Battery Voltage
    DataManager::Battery.voltage = ToVoltage(battery.voltage);

    // Battery Current
    DataManager::Battery.current = ToCurrent(battery.current);

    // Battery Power (32bit)
    uint32_t rawPower =
        ((uint32_t)battery.powerHigh << 16) |
        battery.powerLow;
    
    DataManager::Battery.power = ToPower(rawPower);

    // Battery Status
    DataManager::Battery.status.updated = true;
    DataManager::Battery.status.online = true;
    DataManager::Battery.status.lastUpdate = millis();

    return true;
}

bool Epever::ReadLoad()
{
    EpeverMap::Load load;

    if (!ReadRegisters(
            EpeverRegister::EPEVER_LOAD_VOLTAGE,
            sizeof(load) / sizeof(uint16_t),
            (uint16_t*)&load))
    {
        DataManager::Load.status.updated = false;
        return false;
    }

    // Load Voltage
    DataManager::Load.voltage = ToVoltage(load.voltage);

    // Load Current
    DataManager::Load.current = ToCurrent(load.current);

    // Load Power
    uint32_t rawPower =
        ((uint32_t)load.powerHigh << 16) |
        load.powerLow;
    
    DataManager::Load.power = 
        ToPower(rawPower);

    DataManager::Load.status.updated = true;
    DataManager::Load.status.online = true;
    DataManager::Load.status.lastUpdate = millis();

    return true;
}

bool Epever::ReadTemperature()
{
    EpeverMap::Temperature temp;

    if (!ReadRegisters(
            EpeverRegister::EPEVER_BATTERY_TEMPERATURE,
            sizeof(temp) / sizeof(uint16_t),
            (uint16_t*)&temp))
    {
        DataManager::Temperature.status.updated = false;

        return false;
    }

    DataManager::Temperature.battery = 
        ToTemperature((int16_t)temp.battery);

    DataManager::Temperature.device = 
        ToTemperature((int16_t)temp.battery);
    
    DataManager::Temperature.status.updated = true;
    DataManager::Temperature.status.online = true;
    DataManager::Temperature.status.lastUpdate = millis();

    return true;
}

bool Epever::ReadSOC()
{
    EpeverMap::Soc soc;

    if (!ReadRegisters(
            EpeverRegister::EPEVER_BATTERY_SOC,
            sizeof(soc) / sizeof(uint16_t),
            (uint16_t*)&soc))
    {
        DataManager::Soc.status.updated = false;

        return false;
    }

    DataManager::Soc.value = soc.value;

    DataManager::Soc.status.updated = true;
    DataManager::Soc.status.online = true;
    DataManager::Soc.status.lastUpdate = millis();

    return true;
}

bool Epever::ReadRegisters(
    uint16_t address,
    uint16_t count,
    uint16_t* values)
{
    uint8_t rx[64];

    if (!ModbusRTU::ReadInputRegisters(
        MODBUS_SLAVE_ID,
        address,
        count,
        rx,
        sizeof(rx)))
    {
        return false;
    }

    if (rx[0] != MODBUS_SLAVE_ID)
        return false;
    
    if (rx[1] != MODBUS_READ_INPUT_REGISTERS)
        return false;
    
    if (rx[2] != count * 2)
    {
        Logger::Error(
            "MODBUS",
            "Byte Count Error");
        return false;
    }

    for (uint16_t i = 0; i < count; i++)
    {
        values[i] =
            ((uint16_t)rx[3 + i * 2] << 8) |
            rx[4 + i * 2];
    }
    return true;
}

void Epever::ClearUpdates()
{
    DataManager::Solar.status.updated = false;
    DataManager::Battery.status.updated = false;
    DataManager::Load.status.updated = false;
    DataManager::Temperature.status.updated = false;
    DataManager::Soc.status.updated = false;
}