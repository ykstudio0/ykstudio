//-------------------------------------------------------------
// File : EpeverMaps.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-14
// Project : SVEMS
// Version : 0.2.0
// Descripton : Epever Maps
//-------------------------------------------------------------

#ifndef EPEVER_MAPS_H
#define EPEVER_MAPS_H

#include <Arduino.h>

namespace EpeverMap
{

struct Solar
{
    uint16_t voltage;
    uint16_t current;

    uint16_t powerLow;
    uint16_t powerHigh;
};

struct Battery
{
    uint16_t voltage;
    uint16_t current;
};

struct Load
{
    uint16_t voltage;
    uint16_t current;

    uint16_t powerLow;
    uint16_t powerHigh;
};
}

#endif