//-------------------------------------------------------------
// File : Dag3Monitor.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-10-14
// Project : SVEMS
// Version : 0.8.7
// Description : DAG3 diagnostic request activity monitor
//-------------------------------------------------------------

#include "Dag3Monitor.h"

//-------------------------------------------------------------
// Begin
//-------------------------------------------------------------
void Dag3Monitor::Begin()
{
    m_state = Dag3State::Unknown;

    m_lastRequestMillis = 0;
    m_hasReceivedRequest = false;
}

//-------------------------------------------------------------
// OnRequestReceived
//-------------------------------------------------------------
void Dag3Monitor::OnRequestReceived(uint32_t nowMillis)
{
    m_lastRequestMillis = nowMillis;
    m_hasReceivedRequest = true;

    m_state = Dag3State::Active;
}

//-------------------------------------------------------------
// Update
//-------------------------------------------------------------
void Dag3Monitor::Update(uint32_t nowMillis)
{
    if (!m_hasReceivedRequest)
        return;

    if (m_state == Dag3State::Active &&
        (uint32_t)(nowMillis - m_lastRequestMillis) >=
            REQUEST_TIMEOUT_MS)
    {
        m_state = Dag3State::Inactive;
    }
}

//-------------------------------------------------------------
// GetState
//-------------------------------------------------------------
Dag3State Dag3Monitor::GetState() const
{
    return m_state;
}

//-------------------------------------------------------------
// OnCanFrame
// Description:
//   Detect known DAG3 diagnostic request patterns.
//-------------------------------------------------------------
bool Dag3Monitor::OnCanFrame(
    uint32_t canId,
    const uint8_t* data,
    uint8_t length,
    uint32_t nowMillis)
{
    if (canId != 0x7E0)
        return false;

    if (data == nullptr || length < 3)
        return false;

    // ISO-TP Single Frame
    if ((data[0] & 0xF0) != 0x00)
        return false;

    const uint8_t payloadLength = data[0] & 0x0F;

    if (payloadLength < 2 ||
        payloadLength > 7 ||
        length < payloadLength + 1)
    {
        return false;
    }

    if (data[1] != 0x21)
        return false;

    bool matched = false;

    switch (data[2])
    {
        case 0x01:
        case 0x03:
        case 0x27:
            matched = (payloadLength == 2);
            break;

        case 0x94:
            matched =
                payloadLength == 4 &&
                data[3] == 0x80 &&
                data[4] == 0x01;
            break;

        default:
            break;
    }

    if (!matched)
        return false;

    OnRequestReceived(nowMillis);

    return true;
}