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

void Epever::Begin()
{
    Logger::Info("EPEVER", "Driver Ready");
}

bool Epever::Update()
{
    return true;
}

float Epever::GetPVVoltage()
{
    return 0.0f;
}

float Epever::GetPVCurrent()
{
    return 0.0f;
}

float Epever::GetBatteryVoltage()
{
    return 0.0f;
}

float Epever::GetBatteryCurrent()
{
    return 0.0f;
}

float Epever::GetLoadVoltage()
{
    return 0.0f;
}

float Epever::GetLoadCurrent()
{
    return 0.0f;
}