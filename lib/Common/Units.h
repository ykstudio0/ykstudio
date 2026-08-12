//-----------------------------------------------------------
// File    : Unit.h
// Project : SVEMS
// Author  : JongOh Kim + ChatGPT
// Created : 2026-07-10
//-----------------------------------------------------------

#ifndef UNITS_H
#define UNITS_H

#include <Arduino.h>

//------------------------------------------
// EPEVER Scaling
//------------------------------------------
inline float ToVoltage(uint16_t value)
{
    return value * 0.01f;
}

inline float ToCurrent(uint16_t value)
{
    return value * 0.01f;
}

inline float ToPower(uint32_t value)
{
    return value * 0.01f;
}

inline float ToTemperature(int16_t value)
{

    return value * 0.01f;
}

inline float ToEnergy(uint32_t value)
{
    return value * 10.0f;
}

#endif