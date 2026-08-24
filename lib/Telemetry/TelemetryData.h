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

#include "EpeverStatusParser.h"

namespace SVEMS::Telemetry
{
    enum class TelemetryState : uint8_t
    {
        Online,
        Stale,
        Offline
    };

    struct TimestampData
    {
        uint16_t year = 0U;
        uint8_t month = 0U;
        uint8_t day = 0U;

        uint8_t hour = 0U;
        uint8_t minute = 0U;
        uint8_t second = 0U;
    };

    struct SolarData
    {
        float voltage = 0.0f;
        float current = 0.0f;
        float power = 0.0f;
        float dailyEnergy = 0.0f;
        float totalEnergy = 0.0f;

        EpeverStatusParser::ChargingStage stage =
            EpeverStatusParser::ChargingStage::NotCharging;

        EpeverStatusParser::InputVoltageStatus inputVoltage =
            EpeverStatusParser::InputVoltageStatus::Normal;

        bool online = false;
    };

    struct BatteryData
    {
        float voltage = 0.0f;
        float current = 0.0f;
        float power = 0.0f;

        uint8_t soc = 0U;

        float remainingCapacity = 0.0f;
        float totalCapacity = 0.0f;

        float temperature = 0.0f;

        float cellVoltage[4] =
        {
            0.0f,
            0.0f,
            0.0f,
            0.0f
        };

        bool online = false;
    };

    struct LoadData
    {
        float voltage = 0.0f;
        float current = 0.0f;
        float power = 0.0f;

        bool online = false;
    };

    struct TemperatureData
    {
        float battery = 0.0f;
        float bms = 0.0f;
        float powerBankExternal = 0.0f;

        float controller = 0.0f;
        float controllerBoard = 0.0f;

        TelemetryState cabinState =
            TelemetryState::Offline;

        TelemetryState powerBankState =
            TelemetryState::Offline;

        TelemetryState controllerState =
            TelemetryState::Offline;
    };

    struct EnvironmentData
    {
        float cabinTemperature = 0.0f;
        float cabinHumidity = 0.0f;

        bool online = false;
    };

    struct HttpData
    {
        bool online = false;

        uint8_t state = 0U;

        uint32_t successCount = 0U;
        uint32_t failureCount = 0U;
        uint32_t consecutiveFailures = 0U;
        uint32_t maxConsecutiveFailures = 0U;

        int lastErrorCode = 0;
    };

    struct CommunicationData
    {
        TelemetryState solarState =
            TelemetryState::Offline;
        uint32_t solarTimeoutCount = 0U;
        uint32_t solarOfflineCount = 0U;

        TelemetryState chargeState =
            TelemetryState::Offline;
        uint32_t chargeTimeoutCount = 0U;
        uint32_t chargeOfflineCount = 0U;

        uint32_t bmsOfflineCount = 0U;

        // Modbus error statistics
        uint32_t modbusNoResponseCount = 0U;
        uint32_t modbusFrameTooShortCount = 0U;
        uint32_t modbusInvalidSlaveCount = 0U;
        uint32_t modbusInvalidFunctionCount = 0U;
        uint32_t modbusInvalidLengthCount = 0U;
        uint32_t modbusCrcErrorCount = 0U;
        uint32_t modbusExceptionCount = 0U;
        uint32_t modbusInvalidByteCount = 0U;

        bool rs485Ready = false;
        bool modbusReady = false;

        HttpData http;
    };

    struct SystemData
    {
        uint32_t uptimeSeconds = 0U;

        uint32_t bootCount = 0U;

        const char* resetReason =
            "Unknown";

        int32_t wifiRssi = 0;

        bool wifiConnected = false;

        uint32_t deviceCount = 0U;
        uint32_t deviceTotal = 0U;
    };

    struct VehicleData
    {
        bool active = false;
    };

    struct TelemetryData
    {
        TimestampData timestamp;
        
        SolarData solar;
        BatteryData battery;
        LoadData load;
        TemperatureData temperature;
        EnvironmentData environment;
        CommunicationData communication;
        SystemData system;
        VehicleData vehicle;
    };
}