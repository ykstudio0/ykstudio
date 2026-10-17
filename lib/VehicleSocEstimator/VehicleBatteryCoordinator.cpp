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
// Description:
//   Update parking SOC using validated RTC time.
//-------------------------------------------------------------
void VehicleBatteryCoordinator::Update()
{
    if (!Ready)
    {
        return;
    }

    uint32_t timestamp = 0;

    // RTC time must be valid.
    if (!GetCurrentTimestamp(timestamp))
    {
        return;
    }

    // Only update an existing parking estimation.
    if (BatteryService.GetPowerState() !=
        VehiclePowerState::Parking)
    {
        return;
    }

    // VehicleBatteryService rejects backward timestamps.
    BatteryService.Update(timestamp);
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

//-------------------------------------------------------------
// PrintStatus
// Description:
//   Print vehicle battery SOC estimator status.
//-------------------------------------------------------------
void VehicleBatteryCoordinator::PrintStatus()
{
    if (!Ready)
    {
        Logger::Warning(
            "VEH SOC",
            "Not Ready");

        return;
    }

    const VehiclePowerState state =
        BatteryService.GetPowerState();

    const char* stateText = "Unknown";

    switch (state)
    {
        case VehiclePowerState::EngineOn:
            stateText = "EngineOn";
            break;

        case VehiclePowerState::Parking:
            stateText = "Parking";
            break;

        default:
            break;
    }

    char message[128];

    if (BatteryService.HasIbsSoc())
    {
        snprintf(
            message,
            sizeof(message),
            "State=%s IBS=%.2f%% Estimated=%.2f%%",
            stateText,
            BatteryService.GetIbsSoc(),
            BatteryService.GetEstimatedSoc());
    }
    else
    {
        snprintf(
            message,
            sizeof(message),
            "State=%s IBS=N/A Estimated=N/A",
            stateText);
    }

    Logger::Info(
        "VEH SOC",
        message);
}