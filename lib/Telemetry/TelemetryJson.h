//-------------------------------------------------------------
// File : TelemetryJson.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-09
// Project : SVEMS
// Version : 0.6.0
// Description : Telemetry JSON Serializer
//-------------------------------------------------------------

#pragma once

#include <Arduino.h>

#include "TelemetryData.h"

namespace SVEMS::Telemetry
{
    class TelemetryJson
    {
    public:
        static bool Serialize(
            const TelemetryData& data,
            String& output);
    };
}