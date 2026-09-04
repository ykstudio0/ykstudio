//-------------------------------------------------------------
// File : DisplayPowerManager.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-09-04
// Project : SVEMS
// Version : 0.7.3
// Description : Manages display power states
//-------------------------------------------------------------
#pragma once

#include <stdint.h>

class DisplayPowerManager
{
public:
    enum class State : uint8_t
    {
        Normal = 0,
        Dimmed,
        Off
    };

    static void Begin();

    static void Update();

    static bool NotifyActivity();

    static State GetState();

    static uint8_t GetBrightness();

private:
    static State CurrentState;

    static uint32_t LastActivityMs;
    static uint32_t DimmedStartMs;

    static constexpr uint32_t
        DIM_TIMEOUT_MS =
            5UL * 60UL * 1000UL;

    static constexpr uint32_t
        OFF_DELAY_MS =
            5UL * 1000UL;

    static constexpr uint8_t
        NORMAL_BRIGHTNESS =
            180U;

    static constexpr uint8_t
        DIM_BRIGHTNESS =
            13U;

    static constexpr uint8_t
        OFF_BRIGHTNESS =
            0U;
};