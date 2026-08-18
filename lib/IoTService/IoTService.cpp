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
#include "TelemetryJson.h"
#include "HttpTransport.h"

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

        if (!SVEMS::Transport::HttpTransport::Begin())
        {
            Logger::Error(
                "IOT",
                "HTTP Transport Failed");

            return false;
        }

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

        const auto httpState =
            SVEMS::Transport::
                HttpTransport::GetState();

        m_data.communication.http.online =
            SVEMS::Transport::
                HttpTransport::IsOnline();

        m_data.communication.http.state =
            static_cast<uint8_t>(
                httpState);

        m_data.communication.http.successCount =
            SVEMS::Transport::
                HttpTransport::GetSuccessCount();

        m_data.communication.http.failureCount =
            SVEMS::Transport::
                HttpTransport::GetFailureCount();

        m_data.communication.http.consecutiveFailures =
            SVEMS::Transport::
                HttpTransport::GetConsecutiveFailures();

        m_data.communication.http.lastErrorCode =
            SVEMS::Transport::
                HttpTransport::GetLastErrorCode();

        // Temporary Telemetry Debug Output
        char buffer[128];

        // Solar
        snprintf(
            buffer,
            sizeof(buffer),
            "PV=%.1fV %.1fA %.1fW STAGE=%s INPUT=%s ONLINE=%s",
            m_data.solar.voltage,
            m_data.solar.current,
            m_data.solar.power,
            EpeverStatusParser::ToString(
                m_data.solar.stage),
            EpeverStatusParser::ToString(
                m_data.solar.inputVoltage),
            m_data.solar.online
                ? "YES"
                : "NO");

        Logger::Info(
            "IOT",
            buffer);

        // Battery / BMS
        snprintf(
            buffer,
            sizeof(buffer),
            "BAT=%.2fV %.2fA %.1fW SOC=%u%% ONLINE=%s",
            m_data.battery.voltage,
            m_data.battery.current,
            m_data.battery.power,
            static_cast<unsigned>(
                m_data.battery.soc),
            m_data.battery.online
                ? "YES"
                : "NO");

        Logger::Info(
            "IOT",
            buffer);

        // Environment
        snprintf(
            buffer,
            sizeof(buffer),
            "ENV=%.1fC %.1f%% ONLINE=%s",
            m_data.environment.cabinTemperature,
            m_data.environment.cabinHumidity,
            m_data.environment.online
                ? "YES"
                : "NO");

        Logger::Info(
            "IOT",
            buffer);

        // Wi-Fi
        snprintf(
            buffer,
            sizeof(buffer),
            "WIFI=%s RSSI=%lddBm",
            m_data.system.wifiConnected
                ? "YES"
                : "NO",
            static_cast<long>(
                m_data.system.wifiRssi));

        Logger::Info(
            "IOT",
            buffer);

        // Communication
        snprintf(
            buffer,
            sizeof(buffer),
            "COMM RS485=%s MODBUS=%s",
            m_data.communication.rs485Ready
                ? "YES"
                : "NO",
            m_data.communication.modbusReady
                ? "YES"
                : "NO");

        Logger::Info(
            "IOT",
            buffer);

        // Offline counters
        snprintf(
            buffer,
            sizeof(buffer),
            "OFF SOLAR=%lu CHARGE=%lu BMS=%lu",
            static_cast<unsigned long>(
                m_data.communication.solarOfflineCount),
            static_cast<unsigned long>(
                m_data.communication.chargeOfflineCount),
            static_cast<unsigned long>(
                m_data.communication.bmsOfflineCount));

        Logger::Info(
            "IOT",
            buffer);
            
        // Timeout counters
        snprintf(
            buffer,
            sizeof(buffer),
            "TIMEOUT SOLAR=%lu CHARGE=%lu",
            static_cast<unsigned long>(
                m_data.communication.solarTimeoutCount),
            static_cast<unsigned long>(
                m_data.communication.chargeTimeoutCount));

        Logger::Info(
            "IOT",
            buffer);

        // Temporary JSON Output
        String json;

        if (SVEMS::Telemetry::TelemetryJson::Serialize(
                m_data,
                json))
        {
            Logger::Info(
                "IOT JSON",
                json);

            if (!SVEMS::Transport::HttpTransport::Send(
                    json))
            {
                Logger::Warning(
                    "IOT",
                    "HTTP Send Failed");
            }
        }
        else
        {
            Logger::Warning(
                "IOT JSON",
                "Serialize Failed");
        }
    }

    const SVEMS::Telemetry::TelemetryData&
        IoTService::GetTelemetry()
    {
        return m_data;
    }
}