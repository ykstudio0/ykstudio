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

EpeverData Epever::Data;

uint16_t Epever::Buffer[32];

bool Epever::Update()
{
    bool success = true;

    success &= ReadSolar();
    success &= ReadBattery();
    success &= ReadLoad();
    success &= ReadTemperature();
    success &= ReadSOC();
    
    return success;
}

bool Epever::ReadSolar()
{
    return true;
}

//------------------------------------------
// Read Battery Information
//------------------------------------------
bool Epever::ReadBattery()
{
    uint16_t reg[2];

    //----------------------------------
    // Read Battery Voltage / Current
    //----------------------------------
    if (!ReadRegisters(
        EpeverRegister::EPEVER_BATTERY_VOLTAGE,
        2,
        reg))
    {
        return false;
    }
    
    //------------------------------------
    // Battery Voltage
    //------------------------------------
    Data.batteryVoltage = ToVoltage(reg[0]);

    //------------------------------------
    // Battery Current
    //------------------------------------
    Data.batteryCurrent = ToCurrent(reg[1]);

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

    for (uint16_t i = 0; i < count; i++)
    {
        values[i] =
            ((uint16_t)rx[3 + i * 2] << 8) |
            rx[4 + i * 2];
    }
    return true;
}