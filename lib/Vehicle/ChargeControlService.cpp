//-------------------------------------------------------------
// File : ChargeControlService.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-28
// Project : SVEMS
// Version : 0.7.0
// Description : Vehicle Battery Charge Control
//-------------------------------------------------------------
#include "ChargeControlService.h"

#include <Arduino.h>

#include "Pins.h"
#include "ChargeRelayDriver.h"
#include "Logger.h"

namespace SVEMS
{
    namespace Vehicle
    {
        void ChargeControlService::Begin()
        {
            // IG2 input is already initialized elsewhere.
            // Charge relay remains OFF from ChargeRelayDriver::Begin().
        }

        void ChargeControlService::Update(
            bool enable
        )
        {
            if (
                enable ==
                ChargeRelayDriver::IsEnabled()
            )
            {
                return;
            }

            ChargeRelayDriver::SetEnabled(
                enable
            );

            char buffer[32];

            snprintf(
                buffer,
                sizeof(buffer),
                "ENABLE=%s",
                enable
                    ? "ON"
                    : "OFF"
            );

            Logger::Info(
                "CHARGE",
                buffer
            );
        }

    } // namespace Vehicle
} // namespace SVEMS