//-------------------------------------------------------------
// File : VehicleCanService.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-10-17
// Project : SVEMS Vehicle Battery SOC Estimator
// Version : 0.8.7
// Description : VehicleCanService
//-------------------------------------------------------------

#pragma once

#include <Arduino.h>
#include "VehicleCanReceiver.h"
#include "VehicleEngineMonitor.h"
#include "VehicleIbsParser.h"

class VehicleCanService
{
public:
    static bool Begin();
    static void Update();
    static bool IsReady();
    static void PrintStatus();

    // Engine status
    static VehicleEngineState GetEngineState();
    static float GetEngineRpm();
    static bool IsEngineRpmFresh();

    // IBS SOC
    static bool HasIbsSoc();
    static uint8_t GetIbsSoc();
    static bool IsIbsSocFresh();
    static uint32_t GetLastIbsSocMillis();

private:
    static VehicleCanReceiver Receiver;
    static bool Ready;
    static uint32_t RxCount;
    static VehicleEngineMonitor EngineMonitor;
    static VehicleIbsParser IbsParser;
};