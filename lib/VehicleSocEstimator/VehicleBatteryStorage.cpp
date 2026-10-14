//-------------------------------------------------------------
// File : VehicleBatteryStorage.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-10-14
// Project : SVEMS
// Version : 0.8.7
// Description : SVEMS Vehicle Battery SOC Estimator
//-------------------------------------------------------------

#include "VehicleBatteryStorage.h"
#include <math.h>

//-------------------------------------------------------------
// Save
//-------------------------------------------------------------
bool VehicleBatteryStorage::Save(
    const VehicleBatteryPersistentData& data)
{
    // Validate data before writing
    if (data.version != 1 ||
        !isfinite(data.ibsSoc) ||
        data.ibsSoc < 0.0f ||
        data.ibsSoc > 100.0f)
    {
        return false;
    }

    Preferences prefs;

    if (!prefs.begin(NVS_NAMESPACE, false))
        return false;

    const size_t written =
        prefs.putBytes(
            NVS_KEY,
            &data,
            sizeof(data));

    prefs.end();

    return written == sizeof(data);
}

//-------------------------------------------------------------
// Load
//-------------------------------------------------------------
bool VehicleBatteryStorage::Load(
    VehicleBatteryPersistentData& data)
{
    Preferences prefs;

    // Read-only: do not create or modify namespace
    if (!prefs.begin(NVS_NAMESPACE, true))
        return false;

    if (prefs.getBytesLength(NVS_KEY) !=
        sizeof(VehicleBatteryPersistentData))
    {
        prefs.end();
        return false;
    }

    VehicleBatteryPersistentData loaded{};

    const size_t read =
        prefs.getBytes(
            NVS_KEY,
            &loaded,
            sizeof(loaded));

    prefs.end();

    if (read != sizeof(loaded))
        return false;

    // Validate restored data
    if (loaded.version != 1 ||
        !isfinite(loaded.ibsSoc) ||
        loaded.ibsSoc < 0.0f ||
        loaded.ibsSoc > 100.0f)
    {
        return false;
    }

    // Only accept known power states:
    // Unknown=0, EngineOn=1, Parking=2
    if (loaded.powerState > 2)
        return false;

    // Reject old simulation timestamps.
    // Accept Unix timestamps from 2024-01-01 onward.
    static constexpr uint32_t MIN_VALID_UNIX_TIME =
        1704067200UL;

    if (loaded.lastIbsTimestamp < MIN_VALID_UNIX_TIME)
        return false;

    if (loaded.powerState == 2 &&
        loaded.parkingStartTimestamp < MIN_VALID_UNIX_TIME)
        return false;

    data = loaded;

    return true;
}

//-------------------------------------------------------------
// Clear
//-------------------------------------------------------------
bool VehicleBatteryStorage::Clear()
{
    // Intentionally disabled to protect existing NVS
    return false;
}