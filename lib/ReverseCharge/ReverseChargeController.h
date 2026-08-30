//-------------------------------------------------------------
// File : ReverseChargeController.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-30
// Project : SVEMS
// Version : 0.7.0
// Description : ReverseCharge Management
//-------------------------------------------------------------

#pragma once

#include <Arduino.h>

#include "ReverseChargeTypes.h"

class ReverseChargeController
{
public:

    void Begin();

    void Update(
        const ReverseCharge::Input& input
    );

    void SetMode(
        ReverseCharge::Mode mode
    );

    ReverseCharge::Mode GetMode() const;

    ReverseCharge::State GetState() const;

    bool IsChargeEnabled() const;

    ReverseCharge::SafetyReason GetSafetyReason() const;

private:

    void SetState(
        ReverseCharge::State state
    );

    void StartCharging();

    void StopCharging();

    void ReturnToNormal();

private:

    ReverseCharge::Mode m_mode =
        ReverseCharge::Mode::Normal;

    ReverseCharge::State m_state =
        ReverseCharge::State::Idle;

    ReverseCharge::SafetyReason m_safetyReason =
        ReverseCharge::SafetyReason::None;

    bool m_chargeEnabled = false;

    uint32_t m_stateStartedAt = 0;

    uint32_t m_ig2OffStartedAt = 0;

    uint32_t m_lowVoltageStartedAt = 0;

    uint32_t m_chargeStartedAt = 0;

    uint32_t m_restStartedAt = 0;

    uint32_t m_validVoltageStartedAt = 0;

    bool m_previousIg2Active = false;

    bool m_initialized =
        false;
};