//-------------------------------------------------------------
// File : EpeverStatusParser.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-07-27
// Project : SVEMS
// Version : 0.4.0
// Description : Epever 상태 레지스터 해석
//-------------------------------------------------------------

#include "EpeverStatusParser.h"

namespace EpeverStatusParser
{
    ChargingStatus ParseChargingStatus(uint16_t raw)
    {
        ChargingStatus status{};

        status.raw = raw;

        status.inputVoltage =
            static_cast<InputVoltageStatus>((raw >> 14) & 0x03);

        status.chargingMosfetShort =
            (raw & (1U << 13)) != 0;

        status.chargingMosfetOpen = 
            (raw & (1U << 12)) != 0;

        status.antiReverseMosfetShort =
            (raw & (1U << 11)) != 0;

        status.inputOverCurrent =
            (raw & (1U << 10)) != 0;

        status.loadOverCurrent =
            (raw & (1U << 9)) != 0;

        status.loadShortCircuit =
            (raw & (1U << 8)) != 0;

        status.loadMosfetShort =
            (raw & (1U << 7)) != 0;

        status.circuitDisequilibrium =
            (raw & (1U << 6)) != 0;

        status.pvInputShortCircuit =
            (raw & (1U << 4)) != 0;

        status.stage =
            static_cast<ChargingStage>((raw >> 2) & 0x03);

        status.fault =
            (raw & (1U << 1)) != 0;

        status.running =
            (raw & (1U << 0)) != 0;

        return status;
    }
}