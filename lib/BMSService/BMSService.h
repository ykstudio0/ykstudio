//-------------------------------------------------------------
// File : BMSService.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-06
// Project : SVEMS
// Version : 0.5.7
// Description : PowerBank BMS communication service framework
//-------------------------------------------------------------

#pragma once

#include <Arduino.h>

namespace SVEMS::Service
{
    struct BMSData
    {
        float socPercent;

        float cellVoltage1;
        float cellVoltage2;
        float cellVoltage3;
        float cellVoltage4;

        float packVoltage;
        float packCurrent;

        float batteryTemperature;
        float bmsTemperature;
        float externalTemperature;

        uint32_t cellCount;

        float remainingCapacity;

        BMSData()
            : socPercent(0.0f),
              cellVoltage1(0.0f),
              cellVoltage2(0.0f),
              cellVoltage3(0.0f),
              cellVoltage4(0.0f),
              packVoltage(0.0f),
              packCurrent(0.0f),
              batteryTemperature(0.0f),
              bmsTemperature(0.0f),
              externalTemperature(0.0f),
              cellCount(0U),
              remainingCapacity(0.0f)
        {
        }
    };

    class BMSService
    {
    public:
        static bool Begin();
        static void Update();

        static bool IsReady();
        static bool IsReceiving();
        static bool HasValidData();

        static uint32_t GetLastReceiveTime();

        static const BMSData& GetData();

    private:
        static bool ParseFrame(
            const uint8_t* frame,
            size_t length);

        static bool Ready;
        static bool Receiving;
        static bool ValidData;

        static uint32_t LastReceiveTime;

        static BMSData Data;
    };
}