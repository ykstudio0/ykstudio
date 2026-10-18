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
#include "VehicleCanService.h"
#include "VehicleInput.h"

VehicleBatteryService
    VehicleBatteryCoordinator::BatteryService;

bool VehicleBatteryCoordinator::Ready = false;

bool VehicleBatteryCoordinator::EngineRunningObserved = false;

uint32_t VehicleBatteryCoordinator::LastProcessedIbsMillis = 0;

bool VehicleBatteryCoordinator::EngineStopConfirmed = false;

bool VehicleBatteryCoordinator::CurrentTripIbsReceived = false;

uint32_t VehicleBatteryCoordinator::EngineSessionStartMillis = 0;

uint32_t VehicleBatteryCoordinator::EngineStopConfirmedMillis = 0;

bool VehicleBatteryCoordinator::Ig2OnObserved = false;

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

    EngineStopConfirmed = false;
    CurrentTripIbsReceived = false;

    EngineSessionStartMillis = 0;
    EngineStopConfirmedMillis = 0;

    Ig2OnObserved = false;

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

    if (!GetCurrentTimestamp(timestamp))
    {
        return;
    }

    const VehicleEngineState engineState =
        VehicleCanService::GetEngineState();

    // Start EngineOn only when a valid running engine is confirmed.
    if (engineState == VehicleEngineState::EngineRunning &&
        VehicleCanService::IsEngineRpmFresh())
    {
        if (BatteryService.GetPowerState() !=
            VehiclePowerState::EngineOn)
        {
            BatteryService.StartEngine(timestamp);

            EngineRunningObserved = true;
            CurrentTripIbsReceived = false;
            EngineSessionStartMillis = millis();

            // Reset history for the new engine session
            Ig2OnObserved = false;
            EngineStopConfirmed = false;
            EngineStopConfirmedMillis = 0;

            Logger::Info(
                "VEH SOC",
                "EngineOn Confirmed");

            //-------------------------------------------------------------
            // Invalidate previous Parking snapshot on new EngineOn
            //-------------------------------------------------------------
            VehicleBatteryStorage storage;

            if (storage.InvalidateParking())
            {
                Logger::Info(
                    "VEH SOC",
                    "Previous Parking Invalidated");
            }
            else
            {
                Logger::Warning(
                    "VEH SOC",
                    "Parking Invalidation Failed");
            }
        }

        //-------------------------------------------------------------
        // Record IG2 ON during confirmed engine running
        //-------------------------------------------------------------
        if (VehicleInput::IsActive() &&
            !Ig2OnObserved)
        {
            Ig2OnObserved = true;

            Logger::Info(
                "VEH SOC",
                "IG2 ON Observed");
        }

        // Engine is running again.
        // Any previous stop confirmation is no longer valid.
        if (EngineStopConfirmed)
        {
            EngineStopConfirmed = false;
            EngineStopConfirmedMillis = 0;

            Logger::Info(
                "VEH SOC",
                "EngineStop Cleared");
        }
    }

    // Confirm engine stopped during current driving session
    //-------------------------------------------------------------
    if (BatteryService.GetPowerState() ==
            VehiclePowerState::EngineOn &&
        EngineRunningObserved &&
        engineState == VehicleEngineState::EngineStopped &&
        VehicleCanService::IsEngineRpmFresh())
    {
        if (!EngineStopConfirmed)
        {
            EngineStopConfirmed = true;
            EngineStopConfirmedMillis = millis();

            Logger::Info(
                "VEH SOC",
                "EngineStop Confirmed");
        }
    }

    //-------------------------------------------------------------
    // Process new IBS SOC measurement
    //-------------------------------------------------------------
    if (BatteryService.GetPowerState() ==
            VehiclePowerState::EngineOn &&
        VehicleCanService::HasIbsSoc() &&
        VehicleCanService::IsIbsSocFresh())
    {
        const uint32_t ibsMillis =
            VehicleCanService::GetLastIbsSocMillis();

        // Process each IBS measurement only once.
        if (ibsMillis != LastProcessedIbsMillis)
        {
            // Compare millis timestamps using wrap-safe arithmetic.
            const bool receivedInCurrentTrip =
                static_cast<int32_t>(
                    ibsMillis - EngineSessionStartMillis) >= 0;

            if (receivedInCurrentTrip)
            {
                const bool accepted =
                    BatteryService.ReceiveIbsSoc(
                        static_cast<float>(
                            VehicleCanService::GetIbsSoc()),
                        timestamp);

                if (accepted)
                {
                    LastProcessedIbsMillis = ibsMillis;
                    CurrentTripIbsReceived = true;

                    Logger::Info(
                        "VEH SOC",
                        "New IBS SOC Accepted");
                }
            }
        }
    }

    //-------------------------------------------------------------
    // Start parking after confirmed engine stop and IG2 OFF
    //-------------------------------------------------------------
    if (BatteryService.GetPowerState() ==
            VehiclePowerState::EngineOn &&
        EngineRunningObserved &&
        EngineStopConfirmed &&
        CurrentTripIbsReceived &&
        BatteryService.HasIbsSoc() &&
        Ig2OnObserved &&
        !VehicleInput::IsActive())
    {
        BatteryService.StartParking(timestamp);

        if (BatteryService.GetPowerState() ==
            VehiclePowerState::Parking)
        {
            Logger::Info(
                "VEH SOC",
                "Parking Started");

            //-----------------------------------------------------
            // Save parking baseline to NVS
            //-----------------------------------------------------
            VehicleBatteryPersistentData data{};

            if (BatteryService.ExportPersistentData(data))
            {
                VehicleBatteryStorage storage;

                if (storage.Save(data))
                {
                    Logger::Info(
                        "VEH SOC",
                        "Parking State Saved");
                }
                else
                {
                    Logger::Warning(
                        "VEH SOC",
                        "Parking State Save Failed");
                }
            }
            else
            {
                Logger::Warning(
                    "VEH SOC",
                    "Parking State Export Failed");
            }
        }
    }

    // Update estimated SOC only while Parking.
    if (BatteryService.GetPowerState() ==
        VehiclePowerState::Parking)
    {
        BatteryService.Update(timestamp);
    }
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

    //-------------------------------------------------------------
    // Engine stop diagnostic
    //-------------------------------------------------------------
    char stopMessage[128];

    if (EngineStopConfirmed)
    {
        const uint32_t elapsedMs =
            millis() - EngineStopConfirmedMillis;

        snprintf(
            stopMessage,
            sizeof(stopMessage),
            "Stop=YES Elapsed=%lus TripIBS=%s IG2=%s",
            static_cast<unsigned long>(elapsedMs / 1000UL),
            CurrentTripIbsReceived ? "YES" : "NO",
            VehicleInput::IsActive() ? "ON" : "OFF");
    }
    else
    {
        snprintf(
            stopMessage,
            sizeof(stopMessage),
            "Stop=NO TripIBS=%s IG2=%s",
            CurrentTripIbsReceived ? "YES" : "NO",
            VehicleInput::IsActive() ? "ON" : "OFF");
    }

    Logger::Info(
        "VEH STOP",
        stopMessage);
}

#ifdef VEHICLE_SOC_NVS_TEST

void VehicleBatteryCoordinator::RunNvsTest()
{
    Logger::Info(
        "VEH NVS",
        "Test Save Start");

    //---------------------------------------------------------
    // Create isolated test data
    //---------------------------------------------------------
    VehicleBatteryPersistentData data{};

    data.version = 1;
    data.ibsSoc = 64.0f;
    data.powerState = 2;

    // Fixed test timestamps
    data.lastIbsTimestamp = 1792280000UL;
    data.parkingStartTimestamp = 1792280060UL;

    //---------------------------------------------------------
    // Save to test namespace
    //---------------------------------------------------------
    VehicleBatteryStorage storage;

    if (storage.Save(data))
    {
        Logger::Info(
            "VEH NVS",
            "Test Save Success");
    }
    else
    {
        Logger::Warning(
            "VEH NVS",
            "Test Save Failed");
    }
}

#endif

#ifdef VEHICLE_SOC_NVS_TEST

void VehicleBatteryCoordinator::RunNvsInvalidateTest()
{
    Logger::Info(
        "VEH NVS",
        "Invalidate Test Start");

    VehicleBatteryStorage storage;

    //---------------------------------------------------------
    // Confirm existing Parking data
    //---------------------------------------------------------
    VehicleBatteryPersistentData data{};

    if (!storage.Load(data))
    {
        Logger::Warning(
            "VEH NVS",
            "Test Data Load Failed");
        return;
    }

    if (data.powerState != 2)
    {
        Logger::Warning(
            "VEH NVS",
            "Test Data Not Parking");
        return;
    }

    //---------------------------------------------------------
    // Invalidate Parking data
    //---------------------------------------------------------
    if (!storage.InvalidateParking())
    {
        Logger::Warning(
            "VEH NVS",
            "Invalidate Failed");
        return;
    }

    //---------------------------------------------------------
    // Read back and verify
    //---------------------------------------------------------
    VehicleBatteryPersistentData verify{};

    if (storage.Load(verify) &&
        verify.version == data.version &&
        verify.ibsSoc == data.ibsSoc &&
        verify.lastIbsTimestamp == data.lastIbsTimestamp &&
        verify.parkingStartTimestamp == data.parkingStartTimestamp &&
        verify.powerState == 0)
    {
        Logger::Info(
            "VEH NVS",
            "Invalidate Verified");
    }
    else
    {
        Logger::Warning(
            "VEH NVS",
            "Invalidate Verify Failed");
    }
}

#endif