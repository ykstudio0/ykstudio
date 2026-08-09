//-------------------------------------------------------------
// File : IoTService.cpp
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-09
// Project : SVEMS
// Version : 0.6.0
// Description : IoT Telemetry Service
//-------------------------------------------------------------

#include "IoTService.h"
#include "TelemetryBuilder.h"
#include "Logger.h"

namespace SVEMS::Service
{
    SVEMS::Telemetry::TelemetryData
        IoTService::m_data;

    uint32_t
        IoTService::m_lastUpdate = 0U;

    bool IoTService::Begin()
    {
        SVEMS::Telemetry::TelemetryBuilder::Build(
            m_data);

        m_lastUpdate = millis();

        Logger::Info("IoT", "Ready");

        return true;
    }

    void IoTService::Update()
    {
        const uint32_t now =
            millis();

        if (now - m_lastUpdate <
                UPDATE_INTERVAL_MS)
        {
            return;
        }

        m_lastUpdate = now;

        SVEMS::Telemetry::TelemetryBuilder::Build(
            m_data);

        // Temporary Telemetry Debug Output
        char buffer[96];

        snprintf(
            buffer,
            sizeof(buffer),
            "PV=%.1fV %.1fA %.1fW",
            m_data.solar.voltage,
            m_data.solar.current,
            m_data.solar.power);

        Logger::Info("IOT", buffer);

        snprintf(
            buffer,
            sizeof(buffer),
            "BAT=%.2fV %.2fA %.1fW SOC=%u%%",
            m_data.battery.voltage,
            m_data.battery.current,
            m_data.battery.power,
            static_cast<unsigned>(
                m_data.battery.soc));

        Logger::Info("IOT", buffer);
    }

    const SVEMS::Telemetry::TelemetryData&
        IoTService::GetTelemetry()
    {
        return m_data;
    }
}