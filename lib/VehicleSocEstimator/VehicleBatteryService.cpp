//-------------------------------------------------------------
// File : VehicleBatteryService.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-10-14
// Project : SVEMS
// Version : 0.8.7
// Description : SVEMS Vehicle Battery SOC Estimator
//-------------------------------------------------------------

#include "VehicleBatteryService.h"
#include "VehicleSocEstimator.h"

//-------------------------------------------------------------
// Begin
//-------------------------------------------------------------
void VehicleBatteryService::Begin()
{
    m_ibsSoc = 0.0f;
    m_estimatedSoc = 0.0f;
    m_parkingBaseSoc = 0.0f;

    m_lastIbsTimestamp = 0;
    m_parkingStartTimestamp = 0;
    m_lastUpdateTimestamp = 0;

    m_hasIbsSoc = false;

    m_powerState = VehiclePowerState::Unknown;
}

//-------------------------------------------------------------
// Set IBS SOC
//-------------------------------------------------------------
void VehicleBatteryService::SetIbsSoc(
    float soc,
    uint32_t timestamp)
{
    if (!isfinite(soc) ||
        soc < 0.0f ||
        soc > 100.0f)
    {
        return;
    }

    // Accept IBS measurements only while engine is ON
    if (m_powerState != VehiclePowerState::EngineOn)
        return;

    // Reject older IBS measurements
    if (m_hasIbsSoc &&
        timestamp < m_lastIbsTimestamp)
    {
        return;
    }

    m_ibsSoc = soc;
    m_estimatedSoc = soc;

    m_lastIbsTimestamp = timestamp;
    m_hasIbsSoc = true;
}

//-------------------------------------------------------------
// ReceiveIbsSoc
// Description:
//   Receive a validated IBS SOC measurement.
//
//   Update the service only in EngineOn state.
//   Preserve the parking baseline and NVS v1 compatibility.
//-------------------------------------------------------------
void VehicleBatteryService::ReceiveIbsSoc(
    float soc,
    uint32_t timestamp)
{
    // Reject invalid SOC values.
    if (!isfinite(soc) ||
        soc < 0.0f ||
        soc > 100.0f)
    {
        return;
    }

    // Do not modify parking estimation or unknown state.
    if (m_powerState != VehiclePowerState::EngineOn)
    {
        return;
    }

    // Reuse the existing validation and update logic.
    SetIbsSoc(soc, timestamp);
}

//-------------------------------------------------------------
// Start Engine
//-------------------------------------------------------------
void VehicleBatteryService::StartEngine(
    uint32_t timestamp)
{
    // Parking estimation stops here.
    // Retain the last estimated SOC until a new IBS
    // measurement arrives.

    m_powerState = VehiclePowerState::EngineOn;
}

//-------------------------------------------------------------
// Start Parking
//-------------------------------------------------------------
void VehicleBatteryService::StartParking(
    uint32_t timestamp)
{
    // Parking estimation starts only when leaving EngineOn.
    // Repeated calls must not reset the parking baseline.

    if (m_powerState != VehiclePowerState::EngineOn)
        return;

    // Capture the SOC baseline for parking estimation.
    m_parkingBaseSoc = m_ibsSoc;

    m_parkingStartTimestamp = timestamp;
    m_lastUpdateTimestamp = timestamp;

    m_powerState = VehiclePowerState::Parking;
}

//-------------------------------------------------------------
// Update
//-------------------------------------------------------------
void VehicleBatteryService::Update(
    uint32_t timestamp)
{
    if (!m_hasIbsSoc)
        return;

    if (m_powerState != VehiclePowerState::Parking)
        return;

    // Reject timestamps earlier than parking start
    if (timestamp < m_parkingStartTimestamp)
        return;

    // Reject timestamps older than the last update
    if (timestamp < m_lastUpdateTimestamp)
        return;

    const float elapsedHours =
        (timestamp - m_parkingStartTimestamp) / 3600.0f;

    m_estimatedSoc =
        CalculateVehicleSoc(
            m_parkingBaseSoc,
            elapsedHours);

    // Record the last successfully processed timestamp
    m_lastUpdateTimestamp = timestamp;
}

//-------------------------------------------------------------
// Getters
//-------------------------------------------------------------
float VehicleBatteryService::GetIbsSoc() const
{
    return m_ibsSoc;
}

float VehicleBatteryService::GetEstimatedSoc() const
{
    return m_estimatedSoc;
}

bool VehicleBatteryService::HasIbsSoc() const
{
    return m_hasIbsSoc;
}

VehiclePowerState VehicleBatteryService::GetPowerState() const
{
    return m_powerState;
}

//-------------------------------------------------------------
// ExportPersistentData
// Description:
//   Export current battery state for NVS storage.
//-------------------------------------------------------------
bool VehicleBatteryService::ExportPersistentData(
    VehicleBatteryPersistentData& data) const
{
    // Cannot save without a valid IBS SOC.
    if (!m_hasIbsSoc)
        return false;

    // Only export a confirmed parking state.
    if (m_powerState != VehiclePowerState::Parking)
        return false;

    // Prevent invalid time relationships.
    if (m_lastIbsTimestamp < 1704067200UL ||
        m_parkingStartTimestamp < m_lastIbsTimestamp)
    {
        return false;
    }

    VehicleBatteryPersistentData snapshot{};

    snapshot.version = 1;
    snapshot.ibsSoc = m_ibsSoc;
    snapshot.lastIbsTimestamp = m_lastIbsTimestamp;
    snapshot.parkingStartTimestamp = m_parkingStartTimestamp;
    snapshot.powerState = 2;  // Parking

    data = snapshot;

    return true;
}

//-------------------------------------------------------------
// ImportPersistentData
// Description:
//   Restore a validated parking state.
//   SOC estimation resumes only after Update() receives time.
//-------------------------------------------------------------
bool VehicleBatteryService::ImportPersistentData(
    const VehicleBatteryPersistentData& data)
{
    static constexpr uint32_t MIN_VALID_UNIX_TIME =
        1704067200UL;

    static constexpr uint32_t MAX_VALID_UNIX_TIME =
        2051222400UL;

    // Validate persistent data.
    if (data.version != 1 ||
        !isfinite(data.ibsSoc) ||
        data.ibsSoc < 0.0f ||
        data.ibsSoc > 100.0f ||
        data.powerState != 2 ||
        data.lastIbsTimestamp < MIN_VALID_UNIX_TIME ||
        data.lastIbsTimestamp >= MAX_VALID_UNIX_TIME ||
        data.parkingStartTimestamp < data.lastIbsTimestamp ||
        data.parkingStartTimestamp >= MAX_VALID_UNIX_TIME)
    {
        return false;
    }

    // Restore only after all validations succeed.
    m_ibsSoc = data.ibsSoc;
    m_estimatedSoc = data.ibsSoc;

    // Restore the parking baseline from NVS v1.
    m_parkingBaseSoc = data.ibsSoc;

    m_lastIbsTimestamp = data.lastIbsTimestamp;
    m_parkingStartTimestamp = data.parkingStartTimestamp;
    m_lastUpdateTimestamp = data.parkingStartTimestamp;

    m_hasIbsSoc = true;
    m_powerState = VehiclePowerState::Parking;

    return true;
}

//-------------------------------------------------------------
// GetLastUpdateTimestamp
// Description:
//   Return the last accepted SOC update timestamp.
//-------------------------------------------------------------
uint32_t VehicleBatteryService::GetLastUpdateTimestamp() const
{
    return m_lastUpdateTimestamp;
}