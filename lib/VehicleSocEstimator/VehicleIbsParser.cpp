//-------------------------------------------------------------
// File : VehicleIbsParser.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-10-14
// Project : SVEMS Vehicle Battery SOC Estimator
// Version : 0.8.7
// Description : IBS SOC parser implementation
//-------------------------------------------------------------

#include "VehicleIbsParser.h"

//-------------------------------------------------------------
// Begin
//-------------------------------------------------------------
void VehicleIbsParser::Begin()
{
    m_hasSoc = false;
    m_soc = 0;

    m_lastRequestMillis = 0;
    m_waitingForResponse = false;

    m_responseStarted = false;

    m_expectedSequence = 1;
    m_lastResponseMillis = 0;

    m_responseLength = 0;
    m_candidateSoc = 0;
    m_hasCandidateSoc = false;

    m_lastSocMillis = 0;
}

//-------------------------------------------------------------
// OnCanFrame
// Description:
//   Track IBS request and validate complete ISO-TP response.
//-------------------------------------------------------------
bool VehicleIbsParser::OnCanFrame(
    uint32_t canId,
    const uint8_t* data,
    uint8_t length,
    uint32_t nowMillis)
{
    if (data == nullptr || length < 3)
        return false;

    //---------------------------------------------------------
    // 1. DAG3 diagnostic request
    //---------------------------------------------------------
    if (canId == 0x7E0)
    {
        if (data[0] == 0x02 &&
            data[1] == 0x21 &&
            data[2] == 0x27)
        {
            m_lastRequestMillis = nowMillis;
            m_waitingForResponse = true;
            m_responseStarted = false;
            m_expectedSequence = 1;
            m_lastResponseMillis = 0;
            m_responseLength = 0;
            m_hasCandidateSoc = false;

            return true;
        }

        // Another diagnostic request cancels tracking
        if ((data[0] & 0xF0) == 0x00 &&
            data[1] == 0x21)
        {
            m_waitingForResponse = false;
            m_responseStarted = false;
            m_hasCandidateSoc = false;
        }

        return false;
    }

    //---------------------------------------------------------
    // 2. ECU response
    //---------------------------------------------------------
    if (canId != 0x7E8 || !m_waitingForResponse)
        return false;

    const uint32_t referenceMillis =
        m_responseStarted
            ? m_lastResponseMillis
            : m_lastRequestMillis;

    if ((uint32_t)(nowMillis - referenceMillis) > 1000UL)
    {
        m_waitingForResponse = false;
        m_responseStarted = false;
        m_hasCandidateSoc = false;
        return false;
    }

    //---------------------------------------------------------
    // 3. First Frame
    //---------------------------------------------------------
    if ((data[0] & 0xF0) == 0x10)
    {
        if (length != 8 ||
            data[2] != 0x61 ||
            data[3] != 0x27)
        {
            return false;
        }

        const uint16_t payloadLength =
            ((uint16_t)(data[0] & 0x0F) << 8) |
            data[1];

        // Confirmed vehicle response length
        if (payloadLength != 0x2D)
            return false;

        m_responseLength = payloadLength;
        m_responseStarted = true;
        m_expectedSequence = 1;
        m_lastResponseMillis = nowMillis;
        m_hasCandidateSoc = false;

        return true;
    }

    //---------------------------------------------------------
    // 4. Consecutive Frames
    //---------------------------------------------------------
    if ((data[0] & 0xF0) == 0x20 &&
        m_responseStarted)
    {
        if (length != 8)
            return false;

        const uint8_t sequence = data[0] & 0x0F;

        if (sequence != m_expectedSequence)
        {
            m_waitingForResponse = false;
            m_responseStarted = false;
            m_hasCandidateSoc = false;
            return false;
        }

        //-----------------------------------------------------
        // Capture SOC candidate from CF #2
        //-----------------------------------------------------
        if (sequence == 2)
        {
            if (data[2] > 100)
            {
                m_waitingForResponse = false;
                m_responseStarted = false;
                m_hasCandidateSoc = false;
                return false;
            }

            m_candidateSoc = data[2];
            m_hasCandidateSoc = true;
        }

        m_lastResponseMillis = nowMillis;

        //-----------------------------------------------------
        // Last frame for 45-byte response: CF #6
        //-----------------------------------------------------
        if (sequence == 6)
        {
            if (m_responseLength == 0x2D &&
                m_hasCandidateSoc)
            {
                m_soc = m_candidateSoc;
                m_hasSoc = true;
                m_lastSocMillis = nowMillis;
            }

            m_waitingForResponse = false;
            m_responseStarted = false;
            m_hasCandidateSoc = false;

            return true;
        }

        m_expectedSequence =
            (m_expectedSequence + 1) & 0x0F;

        return true;
    }

    return false;
}

//-------------------------------------------------------------
// HasSoc
//-------------------------------------------------------------
bool VehicleIbsParser::HasSoc() const
{
    return m_hasSoc;
}

//-------------------------------------------------------------
// GetSoc
//-------------------------------------------------------------
uint8_t VehicleIbsParser::GetSoc() const
{
    return m_soc;
}

//-------------------------------------------------------------
// GetLastSocMillis
//-------------------------------------------------------------
uint32_t VehicleIbsParser::GetLastSocMillis() const
{
    return m_lastSocMillis;
}

//-------------------------------------------------------------
// IsSocFresh
// Description:
//   Check whether the last valid IBS SOC is recent.
//-------------------------------------------------------------
bool VehicleIbsParser::IsSocFresh(
    uint32_t nowMillis) const
{
    if (!m_hasSoc)
    {
        return false;
    }

    const uint32_t elapsedMillis =
        (uint32_t)(
            nowMillis - m_lastSocMillis);

    return elapsedMillis <= SOC_FRESH_TIMEOUT_MS;
}