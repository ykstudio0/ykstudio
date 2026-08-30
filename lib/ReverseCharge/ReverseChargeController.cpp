//-------------------------------------------------------------
// File : ReverseChargeController.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-30
// Project : SVEMS
// Version : 0.7.0
// Description : ReverseCharge Management
//-------------------------------------------------------------

#include "ReverseChargeController.h"

using namespace ReverseCharge;

void ReverseChargeController::Begin()
{
    m_mode =
        Mode::Normal;

    m_state =
        State::Idle;

    m_chargeEnabled =
        false;

    m_initialized =
        false;

    m_stateStartedAt =
        millis();

    m_ig2OffStartedAt =
        0;

    m_lowVoltageStartedAt =
        0;

    m_chargeStartedAt =
        0;

    m_restStartedAt =
        0;

    m_validVoltageStartedAt =
        0;

    m_previousIg2Active =
        false;

    m_safetyReason =
        SafetyReason::None;
}

void ReverseChargeController::Update(
    const Input& input
)
{
    const uint32_t now =
        millis();

    if (
        !m_initialized
    )
    {
        m_initialized =
            true;

        m_previousIg2Active =
            input.ig2Active;

        if (
            !input.ig2Active
        )
        {
            m_ig2OffStartedAt =
                now;

            SetState(
                State::WaitAfterIg2Off
            );
        }
    }

    // ---------------------------------------------------------
    // 1. Invalid voltage -> Safety Stop
    // ---------------------------------------------------------

    const bool voltageValid =
        input.voltageValid &&
        input.vehicleVoltage >=
            Config::MIN_VALID_VOLTAGE &&
        input.vehicleVoltage <=
            Config::MAX_VALID_VOLTAGE;

    if (!voltageValid)
    {
        StopCharging();

        m_validVoltageStartedAt =
            0;

        m_safetyReason =
            SafetyReason::InvalidVoltage;

        // Hard mode에서 Safety가 발생하면
        // 강제 충전 세션을 종료하고 Normal로 복귀
        if (
            m_mode ==
            Mode::Hard
        )
        {
            m_mode =
                Mode::Normal;
        }

        SetState(
            State::SafetyStop
        );

        return;
    }

    // ---------------------------------------------------------
    // 1-1. Recover from InvalidVoltage
    // ---------------------------------------------------------

    if (
        m_state ==
            State::SafetyStop &&
        m_safetyReason ==
            SafetyReason::InvalidVoltage
    )
    {
        if (
            m_validVoltageStartedAt == 0
        )
        {
            m_validVoltageStartedAt =
                now;

            return;
        }

        if (
            now -
                m_validVoltageStartedAt <
            Config::VALID_VOLTAGE_RECOVERY_MS
        )
        {
            return;
        }

        m_validVoltageStartedAt =
            0;

        m_safetyReason =
            SafetyReason::None;

        if (
            input.ig2Active
        )
        {
            SetState(
                State::Idle
            );

            m_previousIg2Active =
                true;
        }
        else
        {
            m_ig2OffStartedAt =
                now;

            SetState(
                State::WaitAfterIg2Off
            );

            m_previousIg2Active =
                false;
        }

        return;
    }

    // ---------------------------------------------------------
    // 2. Over-voltage protection while charging
    // ---------------------------------------------------------

    if (
        m_chargeEnabled &&
        input.vehicleVoltage >=
            Config::MAX_CHARGE_VOLTAGE
    )
    {
        StopCharging();

        m_safetyReason =
            SafetyReason::OverVoltage;

        if (
            m_mode ==
            Mode::Hard
        )
        {
            m_mode =
                Mode::Normal;
        }

        SetState(
            State::SafetyStop
        );

        return;
    }

    // 4. OverVoltage latch
    if (
        m_safetyReason ==
        SafetyReason::OverVoltage
    )
    {
        StopCharging();

        if (
            m_state !=
            State::SafetyStop
        )
        {
            SetState(
                State::SafetyStop
            );
        }

        return;
    }

    // ---------------------------------------------------------
    // 3. IG2 ON -> Reverse Charge OFF
    // ---------------------------------------------------------

    if (
        input.ig2Active
    )
    {
        StopCharging();

        m_ig2OffStartedAt =
            0;

        m_lowVoltageStartedAt =
            0;

        // Hard 중이었다면
        // 안전하게 Normal 복귀
        if (
            m_mode ==
            Mode::Hard
        )
        {
            m_mode =
                Mode::Normal;
        }

        SetState(
            State::Idle
        );

        m_previousIg2Active =
            true;

        return;
    }

    // ---------------------------------------------------------
    // 3-1. IG2 ON -> OFF transition
    // ---------------------------------------------------------

    if (
        m_previousIg2Active
    )
    {
        m_ig2OffStartedAt =
            now;

        m_lowVoltageStartedAt =
            0;

        SetState(
            State::WaitAfterIg2Off
        );
    }

    m_previousIg2Active =
        false;

    // ---------------------------------------------------------
    // 4. Normal Mode
    // ---------------------------------------------------------

    if (
        m_mode ==
        Mode::Normal
    )
    {
        // -----------------------------------------------------
        // 4-1. Wait after IG2 OFF
        // -----------------------------------------------------

        if (
            m_state ==
            State::WaitAfterIg2Off
        )
        {
            if (
                m_ig2OffStartedAt == 0
            )
            {
                m_ig2OffStartedAt =
                    now;
            }

            if (
                now -
                    m_ig2OffStartedAt <
                Config::AFTER_IG2_OFF_DELAY_MS
            )
            {
                return;
            }

            SetState(
                State::Idle
            );
        }

        // -----------------------------------------------------
        // 4-2. Low voltage detection
        // -----------------------------------------------------

        if (
            m_state ==
            State::Idle
        )
        {
            if (
                input.vehicleVoltage <=
                Config::NORMAL_START_VOLTAGE
            )
            {
                m_lowVoltageStartedAt =
                    now;

                SetState(
                    State::LowVoltageConfirm
                );

                return;
            }

            return;
        }

        // -----------------------------------------------------
        // 4-3. Low voltage confirmation
        // -----------------------------------------------------

        if (
            m_state ==
            State::LowVoltageConfirm
        )
        {
            // Voltage recovered before confirm time
            if (
                input.vehicleVoltage >
                Config::NORMAL_START_VOLTAGE
            )
            {
                m_lowVoltageStartedAt =
                    0;

                SetState(
                    State::Idle
                );

                return;
            }

            if (
                now -
                    m_lowVoltageStartedAt >=
                Config::LOW_VOLTAGE_CONFIRM_MS
            )
            {
                StartCharging();

                return;
            }

            return;
        }

        // -----------------------------------------------------
        // 4-4. Charging
        // -----------------------------------------------------

        if (
            m_state ==
            State::Charging
        )
        {
            if (
                now -
                    m_chargeStartedAt <
                Config::MIN_CHARGE_TIME_MS
            )
            {
                return;
            }

            StopCharging();

            m_restStartedAt =
                now;

            SetState(
                State::Resting
            );

            return;
        }

        // -----------------------------------------------------
        // 4-5. Resting
        // -----------------------------------------------------

        if (
            m_state ==
            State::Resting
        )
        {
            if (
                now -
                    m_restStartedAt <
                Config::REST_TIME_MS
            )
            {
                return;
            }

            if (
                input.vehicleVoltage >=
                Config::NORMAL_STOP_VOLTAGE
            )
            {
                m_lowVoltageStartedAt =
                    0;

                SetState(
                    State::Idle
                );

                return;
            }

            StartCharging();

            return;
        }
    }

    // ---------------------------------------------------------
    // 5. Soft Mode
    // ---------------------------------------------------------

    if (
        m_mode ==
        Mode::Soft
    )
    {
        // -----------------------------------------------------
        // 5-1. Wait after IG2 OFF
        // -----------------------------------------------------

        if (
            m_state ==
            State::WaitAfterIg2Off
        )
        {
            if (
                m_ig2OffStartedAt == 0
            )
            {
                m_ig2OffStartedAt =
                    now;
            }

            if (
                now -
                    m_ig2OffStartedAt <
                Config::AFTER_IG2_OFF_DELAY_MS
            )
            {
                return;
            }

            SetState(
                State::Idle
            );
        }

        // -----------------------------------------------------
        // 5-2. Emergency low voltage detection
        // -----------------------------------------------------

        if (
            m_state ==
            State::Idle
        )
        {
            if (
                input.vehicleVoltage <=
                Config::SOFT_START_VOLTAGE
            )
            {
                m_lowVoltageStartedAt =
                    now;

                SetState(
                    State::LowVoltageConfirm
                );

                return;
            }

            return;
        }

        // -----------------------------------------------------
        // 5-3. Low voltage confirmation
        // -----------------------------------------------------

        if (
            m_state ==
            State::LowVoltageConfirm
        )
        {
            if (
                input.vehicleVoltage >
                Config::SOFT_START_VOLTAGE
            )
            {
                m_lowVoltageStartedAt =
                    0;

                SetState(
                    State::Idle
                );

                return;
            }

            if (
                now -
                    m_lowVoltageStartedAt >=
                Config::LOW_VOLTAGE_CONFIRM_MS
            )
            {
                StartCharging();

                return;
            }

            return;
        }

        // -----------------------------------------------------
        // 5-4. Charging
        // -----------------------------------------------------

        if (
            m_state ==
            State::Charging
        )
        {
            if (
                now -
                    m_chargeStartedAt <
                Config::MIN_CHARGE_TIME_MS
            )
            {
                return;
            }

            StopCharging();

            m_restStartedAt =
                now;

            SetState(
                State::Resting
            );

            return;
        }

        // -----------------------------------------------------
        // 5-5. Resting
        // -----------------------------------------------------

        if (
            m_state ==
            State::Resting
        )
        {
            if (
                now -
                    m_restStartedAt <
                Config::REST_TIME_MS
            )
            {
                return;
            }

            if (
                input.vehicleVoltage >=
                Config::NORMAL_STOP_VOLTAGE
            )
            {
                m_lowVoltageStartedAt =
                    0;

                SetState(
                    State::Idle
                );

                return;
            }

            StartCharging();

            return;
        }
    }

    // ---------------------------------------------------------
    // 6. Hard Mode
    // ---------------------------------------------------------

    if (
        m_mode ==
        Mode::Hard
    )
    {
        // -----------------------------------------------------
        // 6-1. Manual Stop
        // -----------------------------------------------------

        if (
            input.manualStop
        )
        {
            ReturnToNormal();

            return;
        }

        // -----------------------------------------------------
        // 6-2. Start Hard Charge
        // -----------------------------------------------------

        if (
            m_state !=
            State::Charging
        )
        {
            if (
                input.manualStart
            )
            {
                StartCharging();

                return;
            }

            return;
        }

        // -----------------------------------------------------
        // 6-3. Charging Time Complete
        // -----------------------------------------------------

        if (
            now -
                m_chargeStartedAt >=
            Config::HARD_CHARGE_TIME_MS
        )
        {
            ReturnToNormal();

            return;
        }

        return;
    }
}

void ReverseChargeController::SetMode(
    ReverseCharge::Mode mode
)
{
    if (
        m_state ==
            ReverseCharge::State::SafetyStop &&
        m_safetyReason !=
            ReverseCharge::SafetyReason::None
    )
    {
        m_mode =
            ReverseCharge::Mode::Normal;

        return;
    }

    m_mode =
        mode;
}

Mode ReverseChargeController::GetMode() const
{
    return m_mode;
}

State ReverseChargeController::GetState() const
{
    return m_state;
}

SafetyReason ReverseChargeController::GetSafetyReason() const
{
    return m_safetyReason;
}

bool ReverseChargeController::IsChargeEnabled() const
{
    return m_chargeEnabled;
}

void ReverseChargeController::SetState(
    State state
)
{
    m_state =
        state;

    m_stateStartedAt =
        millis();
}

void ReverseChargeController::StartCharging()
{
    m_chargeEnabled =
        true;

    m_chargeStartedAt =
        millis();

    SetState(
        State::Charging
    );
}

void ReverseChargeController::StopCharging()
{
    m_chargeEnabled =
        false;
}

void ReverseChargeController::ReturnToNormal()
{
    StopCharging();

    m_mode =
        Mode::Normal;

    m_safetyReason =
        SafetyReason::None;

    SetState(
        State::Idle
    );
}
