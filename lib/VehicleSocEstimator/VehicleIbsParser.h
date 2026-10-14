//-------------------------------------------------------------
// File : VehicleIbsParser.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-10-14
// Project : SVEMS Vehicle Battery SOC Estimator
// Version : 0.8.7
// Description : Vehicle IBS SOC response parser
//-------------------------------------------------------------

#pragma once

#include <Arduino.h>

class VehicleIbsParser
{
public:
    void Begin();

    // Process a received CAN frame
    bool OnCanFrame(
        uint32_t canId,
        const uint8_t* data,
        uint8_t length,
        uint32_t nowMillis);

    bool HasSoc() const;
    uint8_t GetSoc() const;

    uint32_t GetLastSocMillis() const;

    bool IsSocFresh(uint32_t nowMillis) const;

private:
    bool m_hasSoc = false;
    uint8_t m_soc = 0;

    uint32_t m_lastRequestMillis = 0;
    bool m_waitingForResponse = false;

    bool m_responseStarted = false;

    // ISO-TP consecutive frame tracking
    uint8_t m_expectedSequence = 1;

    // Last accepted response frame time
    uint32_t m_lastResponseMillis = 0;

    uint16_t m_responseLength = 0;
    uint8_t m_candidateSoc = 0;
    bool m_hasCandidateSoc = false;

    uint32_t m_lastSocMillis = 0;

    // Maximum age of a fresh SOC sample
    static constexpr uint32_t SOC_FRESH_TIMEOUT_MS = 5000UL;
};