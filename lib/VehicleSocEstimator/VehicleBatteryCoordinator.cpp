//-------------------------------------------------------------
// File : VehicleBatteryCoordinator.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-10-17
// Project : SVEMS Vehicle Battery SOC Estimator
// Version : 0.8.7
// Description : Vehicle battery SOC integration coordinator
//-------------------------------------------------------------

#include "VehicleBatteryCoordinator.h"
#include "VehicleBatteryStorage.h"
#include "Logger.h"
#include "TimeService.h"
#include "VehicleTime.h"

VehicleBatteryService
    VehicleBatteryCoordinator::BatteryService;

bool VehicleBatteryCoordinator::Ready = false;

bool VehicleBatteryCoordinator::EngineRunningObserved = false;

uint32_t VehicleBatteryCoordinator::LastProcessedIbsMillis = 0;

//-------------------------------------------------------------
// Begin
//-------------------------------------------------------------
void VehicleBatteryCoordinator::Begin()
{
    if (Ready)
    {
        return;
    }

    BatteryService.Begin();

    EngineRunningObserved = false;
    LastProcessedIbsMillis = 0;

    VehicleBatteryStorage storage;
    VehicleBatteryPersistentData data{};

    if (storage.Load(data))
    {
        if (BatteryService.ImportPersistentData(data))
        {
            Logger::Info(
                "VEH SOC",
                "Parking State Restored");
        }
        else
        {
            Logger::Warning(
                "VEH SOC",
                "Invalid Parking State");
        }
    }
    else
    {
        Logger::Info(
            "VEH SOC",
            "No Saved Parking State");
    }

    Ready = true;
}

//-------------------------------------------------------------
// Update
//-------------------------------------------------------------
void VehicleBatteryCoordinator::Update()
{
    if (!Ready)
    {
        return;
    }

    // Engine / IG2 / IBS / RTC integration
    // will be implemented step by step.
}

//-------------------------------------------------------------
// GetCurrentTimestamp
// Description:
//   Read validated Unix timestamp from SVEMS TimeService.
//-------------------------------------------------------------
bool VehicleBatteryCoordinator::GetCurrentTimestamp(
    uint32_t& timestamp)
{
    if (!SVEMS::Service::TimeService::IsReady())
    {
        return false;
    }

    uint32_t currentTimestamp = 0;

    if (!VehicleTime::ToUnixTimestamp(
            SVEMS::Service::TimeService::Now(),
            currentTimestamp))
    {
        return false;
    }

    // Accept Unix timestamps from 2024-01-01 onward.
    static constexpr uint32_t MIN_VALID_UNIX_TIME =
        1704067200UL;

    if (currentTimestamp < MIN_VALID_UNIX_TIME)
    {
        return false;
    }

    timestamp = currentTimestamp;

    return true;
}