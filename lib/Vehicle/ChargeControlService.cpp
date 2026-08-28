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

        void ChargeControlService::Update()
        {
            const bool ig2Active =
            (digitalRead(PIN_IG2) == LOW);

        const bool enable =
            !ig2Active;

        ChargeRelayDriver::SetEnabled(enable);

        char buffer[48];

        snprintf(
            buffer,
            sizeof(buffer),
            "IG2=%s ENABLE=%s",
            ig2Active ? "ON" : "OFF",
            enable ? "ON" : "OFF");

        Logger::Info(
            "CHARGE",
            buffer);
        }

    } // namespace Vehicle
} // namespace SVEMS