//-------------------------------------------------------------
// File : Epever.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-08
// Project : SVEMS
// Version : 0.1.6
// Descripton : Epever 장치
//-------------------------------------------------------------

#include "Epever.h"
#include "Logger.h"
#include "EpeverRegisters.h"
#include "Modbus.h"
#include "ModbusRTU.h"
#include "Config.h"
#include "Units.h"
#include "EpeverMaps.h"

EpeverData Epever::Data;

bool Epever::Update()
{
    bool ok = true;

    return ok;
}

bool Epever::ReadSolar()
{
    EpeverMap::Solar solar;

    bool updated = false;

    if (!ReadRegisters(
        EpeverRegister::PV_ARRAY_VOLTAGE,
        sizeof(solar) / 2,
        (uint16_t*)&solar))
    {
        return false;
    }

    // PV Voltage
    float voltage = ToVoltage(solar.voltage);

    if (voltage != Data.pvVoltage)
    {
        Data.pvVoltage = voltage;
        updated = true;
    }
    // Data.pvVoltage = ToVoltage(solar.voltage);

    // PV Current
    float current = ToCurrent(solar.current);

    if (current != Data.pvCurrent)
    {
        Data.pvCurrent = current;
        updated = true;
    }
    // Data.pvCurrent = ToCurrent(solar.current);

    // PV Power (32bit)
    uint32_t rawPower =
        ((uint32_t)solar.powerHigh << 16) |
        solar.powerLow;
    
    float power = ToPower(rawPower);

    if (power != Data.pvPower)
    {
        Data.pvPower = power;
        updated = true;
    }
    // Data.pvPower = ToPower(power);

    Data.updated.solar = updated;
    // return true;
}

// Read Battery Information
bool Epever::ReadBattery()
{
    EpeverMap::Battery battery;
    
    // Read Battery Voltage / Current
    if (!ReadRegisters(
        EpeverRegister::EPEVER_BATTERY_VOLTAGE,
        sizeof(battery) / 2,
        (uint16_t*)&battery))
    {
        return false;
    }
    
    bool updated = false;

    // Battery Voltage
    float voltage = ToVoltage(battery.voltage);

    if (voltage != Data.batteryVoltage)
    {
        Data.batteryVoltage = voltage;
        updated = true;
    }
    // Data.batteryVoltage = ToVoltage(battery.voltage);

    // Battery Current
    float current = ToCurrent(battery.current);

    if (current != Data.batteryCurrent)
    {
        Data.batteryCurrent = current;
        updated = true;
    }
    // Data.batteryCurrent = ToCurrent(battery.current);

    Data.updated.battery = updated;
    // return true;
}

bool Epever::Begin()
{
    return true;
}

bool Epever::ReadLoad()
{
    return true;
}

bool Epever::ReadTemperature()
{
    return true;
}

bool Epever::ReadSOC()
{
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