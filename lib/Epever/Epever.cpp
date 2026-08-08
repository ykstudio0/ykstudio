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
#include "EpeverStatusParser.h"

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
    DataManager::Solar.status.online = true;
    DataManager::Solar.status.lastUpdate = millis();

    Logger::Info("SOLAR", "Read OK");

    return true;
}

// Read Battery Information
bool Epever::ReadBattery()
{
    EpeverMap::Battery battery;

    if (!ReadRegisters(
            EpeverRegister::EPEVER_BATTERY_VOLTAGE,
            sizeof(battery) / sizeof(uint16_t),
            reinterpret_cast<uint16_t*>(&battery)))
    {
        DataManager::ControllerBattery
            .status.updated = false;

        return false;
    }

    DataManager::ControllerBattery.voltage =
        ToVoltage(battery.voltage);

    DataManager::ControllerBattery.current =
        ToCurrent(battery.current);

    const uint32_t rawPower =
        (static_cast<uint32_t>(
            battery.powerHigh) << 16) |
        battery.powerLow;

    DataManager::ControllerBattery.power =
        ToPower(rawPower);

    DataManager::ControllerBattery
        .status.updated = true;

    DataManager::ControllerBattery
        .status.online = true;

    DataManager::ControllerBattery
        .status.lastUpdate = millis();

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
            reinterpret_cast<uint16_t*>(&temp)))
    {
        DataManager::Temperature
            .controllerStatus.updated = false;

        return false;
    }

    DataManager::Temperature.controller =
        ToTemperature(
            static_cast<int16_t>(
                temp.battery));

    DataManager::Temperature.controllerBoard =
        ToTemperature(
            static_cast<int16_t>(
                temp.device));

    DataManager::Temperature
        .controllerStatus.updated = true;

    DataManager::Temperature
        .controllerStatus.online = true;

    DataManager::Temperature
        .controllerStatus.lastUpdate = millis();

    return true;
}

bool Epever::ReadSOC()
{
    EpeverMap::Soc soc;

    if (!ReadRegisters(
            EpeverRegister::EPEVER_BATTERY_SOC,
            sizeof(soc) / sizeof(uint16_t),
            reinterpret_cast<uint16_t*>(&soc)))
    {
        return false;
    }

    DataManager::ControllerBattery.soc =
        static_cast<uint8_t>(soc.value);

    return true;
}

bool Epever::ReadChargingStatus()
{
    EpeverMap::ChargingStatus status;

    if (!ReadRegisters(
            EpeverRegister::CHARGING_EQUIPMENT_STATUS,
            sizeof(status) /sizeof(uint16_t),
            (uint16_t*)&status))
    {
        return false;
    }

    char buffer[32];

    snprintf(
        buffer,
        sizeof(buffer),
        "Status = 0x%04X",
        status.value);

    Logger::Info("EPEVER", buffer);

    EpeverStatusParser::ChargingStatus parsed =
        EpeverStatusParser::ParseChargingStatus(status.value);

    DataManager::Charge.stage        = parsed.stage;
    DataManager::Charge.inputVoltage = parsed.inputVoltage;
    DataManager::Charge.running      = parsed.running;
    DataManager::Charge.status.updated = true;
    DataManager::Charge.status.online = true;
    DataManager::Charge.status.lastUpdate = millis();

    snprintf(
        buffer,
        sizeof(buffer),
        "Stage = %s",
        EpeverStatusParser::ToString(parsed.stage));

    Logger::Info("EPEVER", buffer);

    snprintf(
        buffer,
        sizeof(buffer),
        "Running = %s",
        parsed.running ? "YES" : "NO");

    Logger::Info("EPEVER", buffer);

    // snprintf(
    //     buffer,
    //     sizeof(buffer),
    //     "Fault = %s",
    //     parsed.fault ? "YES" : "NO");

    // Logger::Info("EPEVER", buffer);

    snprintf(
        buffer,
        sizeof(buffer),
        "Input = %s",
        EpeverStatusParser::ToString(parsed.inputVoltage));

    Logger::Info("EPEVER", buffer);
    
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
    DataManager::Temperature.controllerStatus.updated = false;
    DataManager::Soc.status.updated = false;
}