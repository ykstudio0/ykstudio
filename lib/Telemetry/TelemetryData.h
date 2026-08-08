//-------------------------------------------------------------
// File : TelemetryData.h
// Author : JongOh Kim + ChatGPT
// Date : 2026-08-09
// Project : SVEMS
// Version : 0.6.0
// Description : SVEMS Telemetry Data Model
//-------------------------------------------------------------

#pragma once

#include <Arduino.h>

namespace SVEMS::Telemetry
{
    struct SolarData
    {
        float voltage = 0.0f;
        float current = 0.0f;
        float power = 0.0f;

        bool online = false;
    };

    struct BatteryData
    {
        float volttage = 0.0f;
        float current = 0.0f;
        float power = 0.0f;

        uint8_t soc = 0U;

        float temperature = 0.0f;

        bool online = false;
    };

    struct EnvironmentData
    {
        float cabinTemperature = 0.0f;
        float cabinHumidity = 0.0f;

        bool online = false;
    };

    struct CommunicationData
    {
        uint32_t solarOfflineCount = 0U;
        uint32_t chargeOfflineCount = 0U;
        uint32_t bmsOfflineCount = 0U;

        bool rs485Read = false;
        bool modbusReady = false;
    };

    struct SystemData
    {
        uint32_t uptimeSeconds = 0U;

        int32_t wifiRssi = 0;

        bool wifiConnected = false;
    };

    struct TelemetryData
    {
        SolarData solar;
        BatteryData battery;
        EnvironmentData environment;
        CommunicationData communication;
        SystemData system;
    };
}