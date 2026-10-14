//-------------------------------------------------------------
// File : VehicleSocEstimator.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-10-14
// Project : SVEMS
// Version : 0.8.7
// Description : SVEMS Vehicle Battery SOC Estimator
//-------------------------------------------------------------

#include <Arduino.h>
#include "VehicleSocEstimator.h"

static constexpr float BATTERY_CAPACITY_AH = 100.0f;

static constexpr float INITIAL_CURRENT_A = 0.182f;
static constexpr float SLEEP_CURRENT_A   = 0.034f;

static constexpr float SLEEP_DELAY_HOURS =
    1280.0f / 3600.0f;


//-------------------------------------------------------------
// Calculate Vehicle Battery SOC
//-------------------------------------------------------------
float CalculateVehicleSoc(
    float initialSoc,
    float elapsedHours)
{
    if (elapsedHours < 0.0f)
        elapsedHours = 0.0f;

    float consumedAh = 0.0f;

    if (elapsedHours <= SLEEP_DELAY_HOURS)
    {
        consumedAh =
            INITIAL_CURRENT_A * elapsedHours;
    }
    else
    {
        consumedAh =
            INITIAL_CURRENT_A * SLEEP_DELAY_HOURS;

        consumedAh +=
            SLEEP_CURRENT_A *
            (elapsedHours - SLEEP_DELAY_HOURS);
    }

    float estimatedSoc =
        initialSoc -
        (consumedAh / BATTERY_CAPACITY_AH) * 100.0f;

    return constrain(
        estimatedSoc,
        0.0f,
        100.0f);
}