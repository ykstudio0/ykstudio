//-----------------------------------------------------
// File    : ModbusRTU.cpp
// Project : SVEMS
// Author  : JongOh Kim + ChatGPT
// Created : 2026-07-06
//-----------------------------------------------------

#include "ModbusRTU.h"
#include "Modbus.h"

#include "CRC16.h"
#include "RS485.h"
#include "Logger.h"

bool ModbusRTU::Begin()
{
    Logger::Info("MODBUS", "Ready");
    
    return true;
}

bool ModbusRTU::ReadHoldingRegisters(
    uint8_t slave,
    uint16_t address,
    uint16_t count)
{
    uint8_t frame[8];

    frame[0] = slave;
    frame[1] = MODBUS_READ_HOLDING_REGISTERS;
    
    frame[2] = address >> 8;
    frame[3] = address & 0xFF;

    frame[4] = count >> 8;
    frame[5] = count & 0xFF;

    uint16_t crc = CRC16::Calculate(frame, 6);

    frame[6] = crc & 0xFF;
    frame[7] = crc >> 8;

    Logger::Hex("TX", frame, sizeof(frame));

    RS485::Send(frame, sizeof(frame));

    return true;
}
