//-------------------------------------------------------------
// File : VehicleBatteryService.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-10-14
// Project : SVEMS
// Version : 0.8.7
// Description : SVEMS Vehicle Battery SOC Estimator
//-------------------------------------------------------------

#pragma once

#include <Arduino.h>
#include "VehicleBatteryStorage.h"

enum class VehiclePowerState
{
    Unknown,    // SOC 추정 대기
    EngineOn,   // IBS SOC 수신 및 갱신
    Parking     //마지막 IBS SOC를 기준으로 방전량 추정
};

class VehicleBatteryService
{
public:
    void Begin();

    void SetIbsSoc(
        float soc,
        uint32_t timestamp);

    bool ReceiveIbsSoc(
        float soc,
        uint32_t timestamp);

    void StartParking(
        uint32_t timestamp);

    void StartEngine(
        uint32_t timestamp);

    void Update(
        uint32_t timestamp);

    float GetIbsSoc() const;
    float GetEstimatedSoc() const;

    bool HasIbsSoc() const;

    VehiclePowerState GetPowerState() const;

    bool ExportPersistentData(
        VehicleBatteryPersistentData& data) const;

    bool ImportPersistentData(
        const VehicleBatteryPersistentData& data);

    uint32_t GetLastUpdateTimestamp() const;

private:
    float m_ibsSoc = 0.0f;
    float m_estimatedSoc = 0.0f;

    // SOC baseline captured when parking estimation begins
    float m_parkingBaseSoc = 0.0f;

    uint32_t m_lastIbsTimestamp = 0;
    uint32_t m_parkingStartTimestamp = 0;
    uint32_t m_lastUpdateTimestamp = 0;

    bool m_hasIbsSoc = false;

    VehiclePowerState m_powerState =
        VehiclePowerState::Unknown;
};