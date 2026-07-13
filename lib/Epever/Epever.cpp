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

// Epever::DataType Epever::Data =
// {
//     0
// };

EpeverData Epever::Data;

bool Epever::Update()
{
    bool ok = true;

    ok &= ReadSolar();
    ok &= ReadBattery();
    // ok &= ReadLoad();
    // ok &= ReadTemperature();
    // ok &= ReadSOC();
    Data.online = ok;
    if (ok)
    {
        Data.lastUpdate = millis();
    }

    return ok;
}

bool Epever::ReadSolar()
{
    uint16_t reg[4];

    if (!ReadRegisters(
        EpeverRegister::PV_ARRAY_VOLTAGE,
        4,
        reg))
    {
        return false;
    }

    // PV Voltage
    Data.pvVoltage = ToVoltage(reg[0]);

    // PV Current
    Data.pvCurrent = ToCurrent(reg[1]);

    // PV Power (32bit)
    uint32_t power =
        ((uint32_t)reg[3] << 16) |
        reg[2];
    
    Data.pvPower = ToPower(power);

    return true;
}

// Read Battery Information
bool Epever::ReadBattery()
{
    uint16_t reg[2];

    // Read Battery Voltage / Current
    if (!ReadRegisters(
        EpeverRegister::EPEVER_BATTERY_VOLTAGE,
        2,
        reg))
    {
        return false;
    }
    
    // Battery Voltage
    Data.batteryVoltage = ToVoltage(reg[0]);

    // Battery Current
    Data.batteryCurrent = ToCurrent(reg[1]);

    return true;
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