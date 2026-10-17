//-------------------------------------------------------------
// File : VehicleBatteryCoordinator.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-10-17
// Project : SVEMS Vehicle Battery SOC Estimator
// Version : 0.8.7
// Description : Vehicle battery SOC integration coordinator
//-------------------------------------------------------------

#pragma once

#include <Arduino.h>

#include "VehicleBatteryService.h"

class VehicleBatteryCoordinator
{
public:
    static void Begin();
    static void Update();

private:
    static VehicleBatteryService BatteryService;

    static bool Ready;
    static bool EngineRunningObserved;

    static uint32_t LastProcessedIbsMillis;

    static bool GetCurrentTimestamp(
        uint32_t& timestamp);
};