//-------------------------------------------------------------
// File : EpeverStatusParser.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-27
// Project : SVEMS
// Version : 0.4.0
// Description : Epever 상태 레지스터 해석
//-------------------------------------------------------------

#pragma once

#include <stdint.h>

namespace EpeverStatusParser
{
    enum class InputVoltageStatus : uint8_t
    {
        Normal       = 0,
        NoInputPower = 1,
        HighVoltage  = 2,
        InputError   = 3
    };

    enum class ChargingStage : uint8_t
    {
        NotCharging = 0,
        Float       = 1,
        Boost       = 2,
        Equalize    = 3
    };

    struct ChargingStatus
    {
        uint16_t raw;

        InputVoltageStatus inputVoltage;
        ChargingStage stage;

        bool chargingMosfetShort;
        bool chargingMosfetOpen;
        bool antiReverseMosfetShort;
        bool inputOverCurrent;
        
        bool loadOverCurrent;
        bool loadShortCircuit;
        bool loadMosfetShort;
        bool circuitDisequilibrium;
        bool pvInputShortCircuit;

        bool fault;
        bool running;
    };

    ChargingStatus ParseChargingStatus(uint16_t raw);
}