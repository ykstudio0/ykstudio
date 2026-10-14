//-------------------------------------------------------------
// File : VehicleBatteryStorage.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-10-14
// Project : SVEMS
// Version : 0.8.7
// Description : SVEMS Vehicle Battery SOC Estimator
//-------------------------------------------------------------

#pragma once

#include <Arduino.h>
#include <Preferences.h>

struct VehicleBatteryPersistentData
{
    uint32_t version = 1;

    float ibsSoc = 0.0f;

    uint32_t lastIbsTimestamp = 0;
    uint32_t parkingStartTimestamp = 0;

    uint8_t powerState = 0;
};

class VehicleBatteryStorage
{
public:
    bool Save(
        const VehicleBatteryPersistentData& data);

    bool Load(
        VehicleBatteryPersistentData& data);

    bool Clear();

private:
    static constexpr const char* NVS_NAMESPACE =
        "sv_vehicle_bat";

    static constexpr const char* NVS_KEY =
        "state";
};