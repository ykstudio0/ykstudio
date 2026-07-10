//-------------------------------------------------------------
// File : EpeverRegisters.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-08
// Project : SVEMS
// Version : 0.1.6
// Descripton : 
//-------------------------------------------------------------
#ifndef EPEVER_REGISTERS_H
#define EPEVER_REGISTERS_H

#include <Arduino.h>

namespace EpeverRegister
{
    //-------------------------------------------------
    // Realtime Data (Input Registers)
    //-------------------------------------------------
    constexpr uint16_t PV_ARRAY_VOLTAGE       = 0x3100;
    constexpr uint16_t PV_ARRAY_CURRENT       = 0x3101;
    constexpr uint16_t PV_ARRAY_POWER_L       = 0x3102;
    constexpr uint16_t PV_ARRAY_POWER_H       = 0x3103;

    constexpr uint16_t EPEVER_BATTERY_VOLTAGE = 0x3104;
    constexpr uint16_t EPEVER_BATTERY_CURRENT = 0x3105;

    constexpr uint16_t EPEVER_LOAD_VOLTAGE    = 0x310C;
    constexpr uint16_t EPEVER_LOAD_CURRENT    = 0x310D;
    constexpr uint16_t EPEVER_LOAD_POWER_L    = 0x310E;
    constexpr uint16_t EPEVER_LOAD_POWER_H    = 0x310F;
}

#endif