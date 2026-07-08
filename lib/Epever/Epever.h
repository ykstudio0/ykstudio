//-------------------------------------------------------------
// File : Epever.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-08
// Project : SVEMS
// Version : 0.1.6
// Descripton : Epever 장치와의 통신을 위한 클래스 구현
//-------------------------------------------------------------

#ifndef EPEVER_H
#define EPEVER_H

#include <Arduino.h>

class Epever
{
public:
    static void Begin();
    static bool Update();
    static float GetPVVoltage();
    static float GetPVCurrent();
    static float GetBatteryVoltage();
    static float GetBatteryCurrent();
    static float GetLoadVoltage();
    static float GetLoadCurrent();
};

#endif