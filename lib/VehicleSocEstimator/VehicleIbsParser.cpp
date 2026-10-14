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
}

//-------------------------------------------------------------
// OnCanFrame
// Description:
//   Track DAG3 IBS request and ECU response start.
//-------------------------------------------------------------
bool VehicleIbsParser::OnCanFrame(
    uint32_t canId,
    const uint8_t* data,
    uint8_t length,
    uint32_t nowMillis)
{
    //---------------------------------------------------------
    // 1. Validate input
    //---------------------------------------------------------
    if (data == nullptr || length < 3)
        return false;

    //---------------------------------------------------------
    // 2. DAG3 diagnostic request (0x7E0)
    //---------------------------------------------------------
    if (canId == 0x7E0)
    {
        // IBS request: 02 21 27
        if (data[0] == 0x02 &&
            data[1] == 0x21 &&
            data[2] == 0x27)
        {
            m_lastRequestMillis = nowMillis;
            m_waitingForResponse = true;
            m_responseStarted = false;

            return true;
        }

        // Another diagnostic request cancels pending IBS
        if ((data[0] & 0xF0) == 0x00 &&
            data[1] == 0x21)
        {
            m_waitingForResponse = false;
            m_responseStarted = false;
        }

        return false;
    }

    //---------------------------------------------------------
    // 3. ECU response First Frame (0x7E8)
    //---------------------------------------------------------
    if (canId == 0x7E8 && m_waitingForResponse)
    {
        // Expected: 10 XX 61 27 ...
        if (length == 8 &&
            (data[0] & 0xF0) == 0x10 &&
            data[2] == 0x61 &&
            data[3] == 0x27)
        {
            const uint16_t payloadLength =
                ((uint16_t)(data[0] & 0x0F) << 8) |
                data[1];

            if (payloadLength > 7)
            {
                m_responseStarted = true;

                m_expectedSequence = 1;
                m_lastResponseMillis = nowMillis;

                return true;
            }
        }
    }

    //---------------------------------------------------------
    // 4. Not a recognized IBS request/response
    //---------------------------------------------------------
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