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
    static constexpr uint32_t MIN_VALID_UNIX_TIME =
        1704067200UL;

    static constexpr uint32_t MAX_VALID_UNIX_TIME =
        2051222400UL;

    //---------------------------------------------------------
    // Validate complete parking data before writing
    //---------------------------------------------------------
    if (data.version != 1 ||
        !isfinite(data.ibsSoc) ||
        data.ibsSoc < 0.0f ||
        data.ibsSoc > 100.0f ||
        data.powerState != 2 ||
        data.lastIbsTimestamp < MIN_VALID_UNIX_TIME ||
        data.lastIbsTimestamp >= MAX_VALID_UNIX_TIME ||
        data.parkingStartTimestamp < data.lastIbsTimestamp ||
        data.parkingStartTimestamp >= MAX_VALID_UNIX_TIME)
    {
        return false;
    }

    //---------------------------------------------------------
    // Write to NVS
    //---------------------------------------------------------
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

bool VehicleBatteryStorage::InvalidateParking()
{
    Preferences prefs;

    //---------------------------------------------------------
    // Open existing NVS namespace
    //---------------------------------------------------------
    if (!prefs.begin(NVS_NAMESPACE, false))
    {
        return false;
    }

    //---------------------------------------------------------
    // Check existing data size
    //---------------------------------------------------------
    const size_t storedSize =
        prefs.getBytesLength(NVS_KEY);

    // No data to invalidate.
    if (storedSize == 0)
    {
        prefs.end();
        return true;
    }

    // Reject unexpected data size.
    if (storedSize != sizeof(VehicleBatteryPersistentData))
    {
        prefs.end();
        return false;
    }

    VehicleBatteryPersistentData data{};

    const size_t read =
        prefs.getBytes(
            NVS_KEY,
            &data,
            sizeof(data));

    if (read != sizeof(data))
    {
        prefs.end();
        return false;
    }

    //---------------------------------------------------------
    // Validate existing data
    //---------------------------------------------------------
    if (data.version != 1 ||
        !isfinite(data.ibsSoc) ||
        data.ibsSoc < 0.0f ||
        data.ibsSoc > 100.0f)
    {
        prefs.end();
        return false;
    }

    // Already invalidated: no additional NVS write required.
    if (data.powerState == 0)
    {
        prefs.end();
        return true;
    }

    // Only Parking data can be invalidated.
    if (data.powerState != 2)
    {
        prefs.end();
        return false;
    }

    //---------------------------------------------------------
    // Mark previous Parking data as invalid
    //---------------------------------------------------------
    data.powerState = 0;

    const size_t written =
        prefs.putBytes(
            NVS_KEY,
            &data,
            sizeof(data));

    prefs.end();

    return written == sizeof(data);
}