//-------------------------------------------------------------
// File : ReverseChargeTypes.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-31
// Project : SVEMS
// Version : 0.7.1
// Description : ReverseCharge Management
//-------------------------------------------------------------
#include "ReverseChargeTypes.h"

namespace ReverseCharge
{
    const char* ToString(
        Mode mode
    )
    {
        switch (
            mode
        )
        {
            case Mode::Normal:
                return "Normal";

            case Mode::Soft:
                return "Soft";

            case Mode::Hard:
                return "Hard";

            default:
                return "Unknown";
        }
    }


    const char* ToString(
        State state
    )
    {
        switch (
            state
        )
        {
            case State::Idle:
                return "Idle";

            case State::WaitAfterIg2Off:
                return "WaitAfterIg2Off";

            case State::LowVoltageConfirm:
                return "LowVoltageConfirm";

            case State::Charging:
                return "Charging";

            case State::Resting:
                return "Resting";

            case State::SafetyStop:
                return "SafetyStop";

            default:
                return "Unknown";
        }
    }

    const char* ToString(
        SafetyReason reason
    )
    {
        switch (
            reason
        )
        {
            case SafetyReason::None:
                return "None";

            case SafetyReason::InvalidVoltage:
                return "InvalidVoltage";

            case SafetyReason::OverVoltage:
                return "OverVoltage";

            default:
                return "Unknown";
        }
    }
}