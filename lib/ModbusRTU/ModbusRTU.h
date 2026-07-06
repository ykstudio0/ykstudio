//---------------------------------------------------------
// File     : ModbusRTU.h
// Project : SVEMS
// Author  : JongOh Kim + ChatGPT
// Created : 2026-07-05
//---------------------------------------------------------

#ifndef MODBUS_RTU_H
#define MODBUS_RTU_H

#include <Arduino.h>

class ModbusRTU
{
public:
    static bool Begin();

    static bool ReadHoldingRegisters(
        uint8_t slave,
        uint16_t address,
        uint16_t count);
};

#endif