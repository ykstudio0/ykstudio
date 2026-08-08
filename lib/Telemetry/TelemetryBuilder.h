//-------------------------------------------------------------
// File : TelemetryBuilder.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-09
// Project : SVEMS
// Version : 0.6.0
// Description : Builds telemetry snapshot from DataManager
//-------------------------------------------------------------

#pragma once
#include "TelemetryData.h"

namespace SVEMS::Telemetry
{
    class TelemetryBuilder
    {
    public:
        static void Build(
            TelemetryData& data);
    };
}