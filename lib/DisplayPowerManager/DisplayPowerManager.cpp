//-------------------------------------------------------------
// File : DisplayPowerManager.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-09-04
// Project : SVEMS
// Version : 0.7.3
// Description : Manages display power states
//-------------------------------------------------------------

#include "DisplayPowerManager.h"

#include <Arduino.h>
#include "Logger.h"

DisplayPowerManager::State
    DisplayPowerManager::CurrentState =
        DisplayPowerManager::State::Normal;

uint32_t
    DisplayPowerManager::LastActivityMs =
        0U;

uint32_t
    DisplayPowerManager::DimmedStartMs =
        0U;

void DisplayPowerManager::Begin()
{
    CurrentState =
        State::Normal;

    LastActivityMs =
        millis();

    DimmedStartMs =
        0U;

    Logger::Info(
        "DISPLAY",
        "Power Manager Ready");
}

void DisplayPowerManager::Update()
{
    const uint32_t now =
        millis();

    switch (CurrentState)
    {
        case State::Normal:
        {
            if (
                now - LastActivityMs >=
                DIM_TIMEOUT_MS
            )
            {
                CurrentState =
                    State::Dimmed;

                DimmedStartMs =
                    now;

                Logger::Info(
                    "DISPLAY",
                    "Dimmed");
            }

            break;
        }

        case State::Dimmed:
        {
            if (
                now - DimmedStartMs >=
                OFF_DELAY_MS
            )
            {
                CurrentState =
                    State::Off;

                Logger::Info(
                    "DISPLAY",
                    "Off");
            }

            break;
        }

        case State::Off:
        default:
            break;
    }
}

bool DisplayPowerManager::NotifyActivity()
{
    const bool wasSleeping =
        CurrentState !=
        State::Normal;

    CurrentState =
        State::Normal;

    LastActivityMs =
        millis();

    DimmedStartMs =
        0U;

    return wasSleeping;
}

DisplayPowerManager::State
DisplayPowerManager::GetState()
{
    return CurrentState;
}

uint8_t DisplayPowerManager::GetBrightness()
{
    switch (CurrentState)
    {
        case State::Normal:
            return NORMAL_BRIGHTNESS;

        case State::Dimmed:
            return DIM_BRIGHTNESS;

        case State::Off:
            return OFF_BRIGHTNESS;

        default:
            return NORMAL_BRIGHTNESS;
    }
}