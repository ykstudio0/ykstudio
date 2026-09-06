//-------------------------------------------------------------
// File : ChargeRelaydriver.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-28
// Project : SVEMS
// Version : 0.7.0
// Description : Vehicle Battery Charge Control
//-------------------------------------------------------------
#include "ChargeRelayDriver.h"

#include <Arduino.h>

#include "Pins.h"

namespace SVEMS
{
    namespace Vehicle
    {
        bool ChargeRelayDriver::m_enabled = false;

        void ChargeRelayDriver::Begin()
        {
            digitalWrite(
                PIN_REVERSE_CHARGE_ENABLE,
                LOW
            );

            pinMode(
                PIN_REVERSE_CHARGE_ENABLE,
                OUTPUT
            );

            m_enabled = false;
        }

        void ChargeRelayDriver::SetEnabled(bool enabled)
        {
            digitalWrite(
                PIN_REVERSE_CHARGE_ENABLE,
                enabled ? HIGH : LOW);

            m_enabled = enabled;
        }

        bool ChargeRelayDriver::IsEnabled()
        {
            return m_enabled;
        }

    } // namespace Vehicle
} // namespace SVEMS