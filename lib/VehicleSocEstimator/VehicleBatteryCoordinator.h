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
    static void PrintStatus();
    #ifdef VEHICLE_SOC_NVS_TEST
        static void RunNvsTest();
        static void RunNvsInvalidateTest();
        static void RunParkingSaveFailureTest();
        static void RunParkingInvalidateFailureTest();
    #endif

private:
    static VehicleBatteryService BatteryService;

    static bool Ready;
    static bool EngineRunningObserved;

    static uint32_t LastProcessedIbsMillis;

    // Confirmed engine stop during the current driving session
    static bool EngineStopConfirmed;

    // Valid IBS SOC received during the current driving session
    static bool CurrentTripIbsReceived;

    static bool GetCurrentTimestamp(
        uint32_t& timestamp);

    // Engine session start time in millis()
    static uint32_t EngineSessionStartMillis;

    // Last confirmed engine stop time (millis)
    static uint32_t EngineStopConfirmedMillis;

    // IG2 ON observed during current engine session
    static bool Ig2OnObserved;

    static bool ParkingInvalidationPending;
    static uint32_t LastParkingInvalidationAttemptMillis;

    static bool ParkingSavePending;

    static uint32_t LastParkingSaveAttemptMillis;
};