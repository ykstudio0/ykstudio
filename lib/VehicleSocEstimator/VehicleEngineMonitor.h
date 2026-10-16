//-------------------------------------------------------------
// File : VehicleEngineMonitor.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-10-16
// Project : SVEMS Vehicle Battery SOC Estimator
// Version : 0.8.7
// Description : Engine running state monitor using CAN RPM
//-------------------------------------------------------------

#pragma once

#include <Arduino.h>

//-------------------------------------------------------------
// Engine operating state
//-------------------------------------------------------------
enum class VehicleEngineState : uint8_t
{
    Unknown = 0,
    EngineStopped,
    EngineRunning
};

//-------------------------------------------------------------
// VehicleEngineMonitor
//-------------------------------------------------------------
class VehicleEngineMonitor
{
public:
    void Begin();

    void OnCanFrame(
        uint32_t canId,
        const uint8_t* data,
        uint8_t length,
        uint32_t nowMillis);

    void Update(uint32_t nowMillis);

    VehicleEngineState GetState() const;

    float GetRpm() const;

    bool IsRpmFresh(uint32_t nowMillis) const;

private:
    static constexpr uint32_t ENGINE_RPM_CAN_ID = 0x316;

    static constexpr float RUNNING_RPM_THRESHOLD = 400.0f;

    static constexpr uint32_t RPM_TIMEOUT_MS = 2000UL;

    static constexpr uint32_t MAX_FRAME_GAP_MS = 200UL;

    static constexpr uint32_t STATE_CONFIRM_MS = 1000UL;

    VehicleEngineState m_state =
        VehicleEngineState::Unknown;

    float m_rpm = 0.0f;

    uint32_t m_lastRpmMillis = 0;

    uint32_t m_previousRpmMillis = 0;
    bool m_hasContinuousRpm = false;

    uint32_t m_candidateStartMillis = 0;

    VehicleEngineState m_candidateState =
        VehicleEngineState::Unknown;

    bool m_hasRpm = false;
};