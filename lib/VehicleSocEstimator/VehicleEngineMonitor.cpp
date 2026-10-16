//-------------------------------------------------------------
// File : VehicleEngineMonitor.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-10-16
// Project : SVEMS Vehicle Battery SOC Estimator
// Version : 0.8.7
// Description : Engine state detection using CAN RPM
//-------------------------------------------------------------

#include "VehicleEngineMonitor.h"

//-------------------------------------------------------------
// Begin
//-------------------------------------------------------------
void VehicleEngineMonitor::Begin()
{
    m_state = VehicleEngineState::Unknown;
    m_candidateState = VehicleEngineState::Unknown;

    m_rpm = 0.0f;
    m_lastRpmMillis = 0;
    m_previousRpmMillis = 0;
    m_hasContinuousRpm = false;
    m_candidateStartMillis = 0;
    m_hasRpm = false;
}

//-------------------------------------------------------------
// OnCanFrame
//-------------------------------------------------------------
void VehicleEngineMonitor::OnCanFrame(
    uint32_t canId,
    const uint8_t* data,
    uint8_t length,
    uint32_t nowMillis)
{
    if (canId != ENGINE_RPM_CAN_ID ||
        data == nullptr ||
        length < 4)
    {
        return;
    }

    // Little-endian 16-bit RPM value
    const uint16_t rawRpm =
        static_cast<uint16_t>(data[2]) |
        (static_cast<uint16_t>(data[3]) << 8);

    m_rpm = static_cast<float>(rawRpm) * 0.25f;

    // Check the gap between valid RPM frames.
    const bool frameGapValid =
        m_hasRpm &&
        (uint32_t)(nowMillis - m_lastRpmMillis)
            <= MAX_FRAME_GAP_MS;

    if (!frameGapValid)
    {
        //-----------------------------------------------------
        // Long frame gap invalidates the previous engine state
        //-----------------------------------------------------
        m_state = VehicleEngineState::Unknown;

        // A long gap breaks the confirmation sequence.
        m_candidateState = VehicleEngineState::Unknown;
        m_candidateStartMillis = nowMillis;
        m_hasContinuousRpm = false;
    }
    else
    {
        m_hasContinuousRpm = true;
    }

    m_previousRpmMillis = m_lastRpmMillis;
    m_lastRpmMillis = nowMillis;
    m_hasRpm = true;
}

//-------------------------------------------------------------
// Update
//-------------------------------------------------------------
void VehicleEngineMonitor::Update(uint32_t nowMillis)
{
    //---------------------------------------------------------
    // RPM communication protection
    //---------------------------------------------------------
    if (!IsRpmFresh(nowMillis) ||
        (uint32_t)(nowMillis - m_lastRpmMillis)
            > MAX_FRAME_GAP_MS)
    {
        m_state = VehicleEngineState::Unknown;

        m_candidateState = VehicleEngineState::Unknown;
        m_candidateStartMillis = nowMillis;

        m_hasContinuousRpm = false;

        return;
    }

    //---------------------------------------------------------
    // Determine candidate state
    //---------------------------------------------------------
    VehicleEngineState newCandidate =
        VehicleEngineState::Unknown;

    if (m_rpm >= RUNNING_RPM_THRESHOLD)
    {
        newCandidate = VehicleEngineState::EngineRunning;
    }
    else if (m_rpm == 0.0f)
    {
        newCandidate = VehicleEngineState::EngineStopped;
    }

    //---------------------------------------------------------
    // Intermediate RPM values are inconclusive
    //---------------------------------------------------------
    if (newCandidate == VehicleEngineState::Unknown)
    {
        m_state = VehicleEngineState::Unknown;
        m_candidateState = VehicleEngineState::Unknown;
        return;
    }

    //---------------------------------------------------------
    // Start confirmation timer on candidate change
    //---------------------------------------------------------
    if (newCandidate != m_candidateState)
    {
        //-----------------------------------------------------
        // Invalidate previous confirmed state immediately
        //-----------------------------------------------------
        m_state = VehicleEngineState::Unknown;

        m_candidateState = newCandidate;
        m_candidateStartMillis = nowMillis;

        return;
    }

    //---------------------------------------------------------
    // Confirm only when a recent RPM frame was received
    //---------------------------------------------------------
    const uint32_t rpmAge =
        (uint32_t)(nowMillis - m_lastRpmMillis);

    static constexpr uint32_t CONFIRM_FRAME_MAX_AGE_MS = 200UL;

    if (rpmAge > CONFIRM_FRAME_MAX_AGE_MS)
    {
        return;
    }

    //---------------------------------------------------------
    // Confirm state after stable interval
    //---------------------------------------------------------
    if (m_hasContinuousRpm &&
        (uint32_t)(nowMillis - m_candidateStartMillis)
            >= STATE_CONFIRM_MS)
    {
        m_state = newCandidate;
    }
}

//-------------------------------------------------------------
// GetState
//-------------------------------------------------------------
VehicleEngineState VehicleEngineMonitor::GetState() const
{
    return m_state;
}

//-------------------------------------------------------------
// GetRpm
//-------------------------------------------------------------
float VehicleEngineMonitor::GetRpm() const
{
    return m_rpm;
}

//-------------------------------------------------------------
// IsRpmFresh
//-------------------------------------------------------------
bool VehicleEngineMonitor::IsRpmFresh(
    uint32_t nowMillis) const
{
    if (!m_hasRpm)
    {
        return false;
    }

    return (uint32_t)(nowMillis - m_lastRpmMillis)
        <= RPM_TIMEOUT_MS;
}