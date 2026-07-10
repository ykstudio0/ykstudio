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

struct EpeverData
{
    //-----------------------------------------
    // Solar
    //-----------------------------------------
    float pvVoltage = 0.0f;
    float pvCurrent = 0.0f;
    float pvPower = 0.0f;

    //-----------------------------------------
    // Battery
    //-----------------------------------------
    float batteryVoltage = 0.0f;
    float batteryCurrent = 0.0f;
    float batteryPower = 0.0f;

    uint8_t batterySOC = 0;

    //-----------------------------------------
    // Load
    //-----------------------------------------
    float loadVoltage = 0.0f;
    float loadCurrent = 0.0f;
    float loadPower = 0.0f;

    //-----------------------------------------
    // Temperature
    //-----------------------------------------
    float batteryTemperature = 0.0f;
    float controllerTemperature = 0.0f;
};

class Epever
{
public:
    static bool Begin();
    static bool Update();
    static EpeverData Data;

private:
    static uint16_t Buffer[32];    
    static bool ReadSolar();
    static bool ReadBattery();
    static bool ReadLoad();
    static bool ReadTemperature();
    static bool ReadSOC();

    static bool ReadRegisters(
        uint16_t address,
        uint16_t count,
        uint16_t* values
    );
};

#endif