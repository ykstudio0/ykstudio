//-------------------------------------------------------------
// File : VehicleInput.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-20
// Project : SVEMS
// Version : 0.6.3
// Description : VehicleInput
//-------------------------------------------------------------

#include "VehicleInput.h"
#include "DataManager.h"
#include "Logger.h"
#include "Pins.h"

bool VehicleInput::Active = false;

bool VehicleInput::Begin()
{
    pinMode(
        PIN_IG2,
        INPUT_PULLUP
    );

    Active = false;

    Logger::Info(
        "VEHICLE",
        "Ready"
    );

    return true;
}

void VehicleInput::Update()
{
    const bool active =
        digitalRead(
            PIN_IG2
        ) == LOW;

    if (active != Active)
    {
        Active = active;

        Logger::Info(
            "VEHICLE",
            Active
                ? "IG2 ON"
                : "IG2 OFF"
        );
    }

    DataManager::Vehicle.active =
        Active;
}

bool VehicleInput::IsActive()
{
    return Active;
}