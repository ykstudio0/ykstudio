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

        // status.fault =
        //     (raw & (1U << 1)) != 0;

        // D6,D5,D1,D0
        // Meaning not yet verified on actual hardware.
        status.statusFlags =
            raw & 0x63;

        status.running =
            (raw & (1U << 0)) != 0;

        return status;
    }

    const char* ToString(ChargingStage stage)
    {
        switch (stage)
        {
            case ChargingStage::NotCharging:
                return "Not Charging";

            case ChargingStage::Float:
                return "Float";

            case ChargingStage::Boost:
                return "Boost";

            case ChargingStage::Equalize:
                return "Equalize";

            default:
                return "Unknown";
        }
    }

    const char* ToString(InputVoltageStatus status)
    {
        switch (status)
        {
            case InputVoltageStatus::Normal:
                return "Normal";

            case InputVoltageStatus::NoInputPower:
                return "No PV Input";

            case InputVoltageStatus::HighVoltage:
                return "High Voltage";

            case InputVoltageStatus::InputError:
                return "Input Error";

            default:
                return "Unknown";
        }
    }
}