//-------------------------------------------------------------
// File : IoTService.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-09
// Project : SVEMS
// Version : 0.6.0
// Description : IoT Telemetry Service
//-------------------------------------------------------------

#pragma once

#include <Arduino.h>
#include "TelemetryData.h"

namespace SVEMS::Service
{
    class IoTService
    {
    public:
        static bool Begin();

        static void Update();

        static const SVEMS::Telemetry::TelemetryData&
            GetTelemetry();

    private:
        static SVEMS::Telemetry::TelemetryData m_data;

        static uint32_t m_lastUpdate;

        static constexpr uint32_t 
            UPDATE_INTERVAL_MS = 5000UL;
    };
}